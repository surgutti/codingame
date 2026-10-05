#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <omp.h>
#include <algorithm>
#include <cmath>
#include <vector>

#include "src/engine.h"

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
      writeRawState(envs_[i], state0_ptr + i * RAW_STATE_DIM, 0);
      writeRawState(envs_[i], state1_ptr + i * RAW_STATE_DIM, 1);
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
        writeRawState(envs_[i], state0_ptr + i * RAW_STATE_DIM, 0);
        writeRawState(envs_[i], state1_ptr + i * RAW_STATE_DIM, 1);
      }
    }
  }

private:

  static void writeRawState(Engine const& eng, f32* out, int team) {
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
