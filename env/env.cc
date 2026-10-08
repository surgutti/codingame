#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <omp.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>

#include "src/engine.h"
#include "src/unit.h"

namespace nb = nanobind;

class VectorEnv {
public:
  VectorEnv(i32 num_envs, i64 seed=42)
    : num_envs_(num_envs),
      base_seed_(seed),
      envs_(num_envs),
      episode_counts_(num_envs, 0) {
  }

  void reset(nb::ndarray<f32, nb::c_contig, nb::device::cpu> state0_out,
             nb::ndarray<f32, nb::c_contig, nb::device::cpu> state1_out) {
    f32* state0_ptr = state0_out.data();
    f32* state1_ptr = state1_out.data();
    #pragma omp parallel for schedule(static)
    for (i32 i = 0; i < num_envs_; i++) {
      i64 env_seed = base_seed_ + static_cast<i64>(i) * 1000000LL + (episode_counts_[i]++);
      envs_[i].initializeRefereeGenerated(LAPS, env_seed);
      writeObservations(envs_[i], state0_ptr + i * RAW_STATE_DIM, 0);
      writeObservations(envs_[i], state1_ptr + i * RAW_STATE_DIM, 1);
    }
  }

  void step(nb::ndarray<const f32, nb::c_contig, nb::device::cpu> actions_in,
            nb::ndarray<f32, nb::c_contig, nb::device::cpu> state0_out,
            nb::ndarray<f32, nb::c_contig, nb::device::cpu> state1_out,
            nb::ndarray<f32, nb::c_contig, nb::device::cpu> rewards_out,
            nb::ndarray<f32, nb::c_contig, nb::device::cpu> dones_out) {
    const f32* act_ptr = actions_in.data();
    f32* state0_ptr = state0_out.data();
    f32* state1_ptr = state1_out.data();
    f32* rew_ptr = rewards_out.data();
    f32* done_ptr = dones_out.data();

    {
      nb::gil_scoped_release reelase;
      #pragma omp parallel for schedule(static)
      for (i32 i = 0; i < num_envs_; i++) {
        const f32* env_act =  act_ptr + i * (POD_NB * 4);
        for (i32 podId = 0; podId < POD_NB; podId++) {
          const f32* a = env_act + podId * 4;
          Pod const& pod = envs_[i].pod(podId);

          Move move{};
          f64 base_angle = fabs(pod.angle - START_ANGLE) < 1e-9
            ? pod.getAngle(envs_[i].checkpoints()[pod.next % envs_[i].checkpoints().size()])
            : pod.angle;
          f64 target_angle = base_angle + std::clamp(static_cast<f64>(a[0]), -MAX_ROTATION, +MAX_ROTATION);

          move.target.x = std::floor(pod.x + std::cos(target_angle) * 5000 + 0.5);
          move.target.y = std::floor(pod.y + std::sin(target_angle) * 5000 + 0.5);
          move.thrust   = std::clamp(static_cast<i32>(std::lround(a[1])), 0, MAX_THRUST);
          move.shield   = (a[2] > 0.5f);
          move.boost    = (a[3] > 0.5f);

          envs_[i].applyMove(podId, move);
        }

        rew_ptr[i] = envs_[i].nextTurn();
        if (envs_[i].winnerTeam() != -1) {
          done_ptr[i] = static_cast<f32>(envs_[i].winnerTeam() + 1);
          i64 env_seed = base_seed_ + static_cast<i64>(i) * 1000000LL + (episode_counts_[i]++);
          envs_[i].initializeRefereeGenerated(LAPS, env_seed);
        } else {
          done_ptr[i] = 0.0f;
        }
        writeObservations(envs_[i], state0_ptr + i * RAW_STATE_DIM, 0);
        writeObservations(envs_[i], state1_ptr + i * RAW_STATE_DIM, 1);
      }
    }
  }

private:

  static void writeObservations(Engine const& eng, f32* out, int team) {
    auto const& pods = eng.pods();
    auto const& cps = eng.checkpoints();
    i32 cps_len = static_cast<i32>(cps.size());
    for (i32 p = 0; p < POD_NB; p++) {
      i32 q = (team == 0 ? p : (p ^ 2));
      out[p * 16 + 0] = static_cast<f32>(pods[q].x);
      out[p * 16 + 1] = static_cast<f32>(pods[q].y);
      out[p * 16 + 2] = static_cast<f32>(pods[q].vx);
      out[p * 16 + 3] = static_cast<f32>(pods[q].vy);
      out[p * 16 + 4] = static_cast<f32>(pods[q].angle);
      out[p * 16 + 5] = static_cast<f32>(pods[q].next);
      out[p * 16 + 6] = static_cast<f32>(pods[q].shield);
      out[p * 16 + 7] = static_cast<f32>(pods[q].boosted);

      for (i32 cpId = 0; cpId < 4; cpId++) {
        Checkpoint const& cp = cps[(pods[q].next + cpId) % cps_len];
        out[p * 16 + 8 + cpId * 2 + 0] = static_cast<f32>(cp.x);
        out[p * 16 + 8 + cpId * 2 + 1] = static_cast<f32>(cp.y);
      }
    }
    for (i32 c = 0; c < MAX_CP; c++) {
      out[64 + c * 2 + 0] = static_cast<f32>(cps[c % cps_len].x);
      out[64 + c * 2 + 1] = static_cast<f32>(cps[c % cps_len].y);
    }
    out[76] = static_cast<f32>(cps_len);
    out[77] = static_cast<f32>(LAPS);
    out[78] = static_cast<f32>(eng.timeouts()[0 ^ team]);
    out[79] = static_cast<f32>(eng.timeouts()[1 ^ team]);

    // from here only the neural network features
    i32 nxt = 80;
    for (i32 p = 0; p < 2; p++) {
      i32 a = p ^ (team << 1);

      Vector af{cos(pods[a].angle), sin(pods[a].angle)};
      Vector av{pods[a].vx, pods[a].vy};

      f32 arv = std::max<f32>(hypot(av.x, av.y), 1e-6);
      Vector auv{av.x / arv, av.y / arv};

      // TODO: think about better function for speed
      out[nxt++] = 1.0 / (1 + arv / 500.0);
      out[nxt++] = static_cast<f32>(pods[a].shield) / 3.0;
      out[nxt++] = static_cast<f32>(pods[a].boosted);

      for (i32 o = 1; o < 4; o++) {
        i32 b = a ^ o; // vectors to
      
        Checkpoint cp0 = cps[pods[b].next % cps_len],
                   cp1 = cps[(pods[b].next + 1) % cps_len],
                   cp2 = cps[(pods[b].next + 2) % cps_len];

        Vector bd{pods[b].x - pods[a].x, pods[b].y - pods[a].y};
        Vector bv{pods[b].vx, pods[b].vy};
        Vector bdv{pods[b].x + pods[b].vx - pods[a].x,
                   pods[b].y + pods[b].vy - pods[a].y};
        Vector bc0{cp0.x - pods[a].x, cp0.y - pods[a].y};
        Vector bc1{cp1.x - pods[a].x, cp1.y - pods[a].y};
        Vector bc2{cp2.x - pods[a].x, cp2.y - pods[a].y};
        Vector bc01{cp1.x - cp0.x, cp1.y - cp0.y};
        
        f32 brd = std::max<f32>(hypot(bd.x, bd.y), 1e-6);
        f32 brv = std::max<f32>(hypot(bv.x, bv.y), 1e-6);
        f32 brdv = std::max<f32>(hypot(bdv.x, bdv.y), 1e-6);
        f32 brc0 = std::max<f32>(hypot(bc0.x, bc0.y), 1e-6);
        f32 brc1 = std::max<f32>(hypot(bc1.x, bc1.y), 1e-6);
        f32 brc2 = std::max<f32>(hypot(bc2.x, bc2.y), 1e-6);
        f32 brc01 = std::max<f32>(hypot(bc01.x, bc01.y), 1e-6);
        
        Vector bf{cos(pods[b].angle), sin(pods[b].angle)}; 
        Vector bud{bd.x / brd, bd.y / brd};
        Vector buv{bv.x / brv, bv.y / brv};
        Vector budv{bdv.x / brdv, bdv.y / brdv};
        Vector buc0{bc0.x / brc0, bc0.y / brc0};
        Vector buc1{bc1.x / brc1, bc1.y / brc1};
        Vector buc2{bc2.x / brc2, bc2.y / brc2};
        Vector buc01{bc01.x / brc01, bc01.y / brc01};

        out[nxt++] = 1.0 / (1 + brv  / 500.0);
        out[nxt++] = 1.0 / (1 + brd  / 500.0);
        out[nxt++] = 1.0 / (1 + brdv / 500.0);
        out[nxt++] = 1.0 / (1 + brc0 / 500.0);
        out[nxt++] = 1.0 / (1 + brc1 / 500.0);
        out[nxt++] = 1.0 / (1 + brc2 / 500.0);
        out[nxt++] = 1.0 / (1 + brc01 / 500.0);

        for (Vector av : {af, auv}) {
          for (Vector bv : {bf, bud, buv, bdv, buc0, buc1, buc2, buc01}) {
            out[nxt++] = av.dot(bv);
            out[nxt++] = av.cross(bv);
          }
        }

        out[nxt++] = static_cast<f32>(pods[a].next - pods[b].next) / cps_len / LAPS;
        out[nxt++] = std::min<f32>(2.0, pods[a].collisionTime(pods[b], POD_DIAMETER_SQ));
      }

      for (i32 c = 0; c < 3; c++) {
        Checkpoint cp = cps[(pods[a].next + c) % cps_len];
        Unit unit_cp(cp.x, cp.y);

        Vector d{cp.x - pods[a].x, cp.y - pods[a].y};
        f32 rd = std::max<f32>(hypot(d.x, d.y), 1e-6);
        
        Vector ud{d.x / rd, d.y / rd};

        out[nxt++] = 1.0 / (1 + rd / 500.0);
        out[nxt++] = std::min<f32>(2.0, pods[a].collisionTime(unit_cp, POD_AND_CHECKPOINT_SQ));

        for (Vector av : {af, auv}) {
          for (Vector cv : {ud}) {
            out[nxt++] = av.dot(cv);
            out[nxt++] = av.cross(cv);
          }
        }
      }
    }

    out[nxt++] = static_cast<f32>(eng.timeouts()[0 ^ team]) / 100.0;
    out[nxt++] = static_cast<f32>(eng.timeouts()[1 ^ team]) / 100.0;

    // std::cerr << "NXT: " << nxt << '\n';
  }

  i32 num_envs_;
  i64 base_seed_;
  std::vector<Engine> envs_;
  std::vector<i64> episode_counts_;
};

NB_MODULE(engine, m) {
  nb::class_<VectorEnv>(m, "VectorEnv")
    .def(nb::init<i32, i64>())
    .def("reset", &VectorEnv::reset)
    .def("step", &VectorEnv::step);
}
