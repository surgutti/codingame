#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <torch/extension.h>
#include <omp.h>
#include <algorithm>
#include <cmath>
#include <vector>

#include "sim/engine.h"

namespace nb = nanobind;

class VectorEnv {
public:
  VectorEnv(i32 num_envs, i64 seed=42)
    : num_envs_(num_envs),
      base_seed_(seed),
      envs_(num_envs),
      episode_counts_(num_envs, 0) {
    auto opts = torch::TensorOptions().dtype(torch::kFloat32).pinned_memory(true);
    raw_state_cpu_ = torch::zeros({num_envs, RAW_STATE_DIM}, opts);
    rewards_cpu_   = torch::zeros({num_envs}, opts);
    dones_cpu_     = torch::zeros({num_envs}, opts);
    reset();
  }

  torch::Tensor reset() {
    f32* state_ptr = raw_state_cpu_.data_ptr<f32>();
    #pragma omp parallel for schedule(static)
    for (i32 i = 0; i < num_envs_; i++) {
      i64 env_seed = base_seed_ + static_cast<i64>(i) * 1000000LL + (episode_counts_[i]++);
      envs_[i].initializeRefereeGenerated(LAPS, env_seed);
      writeRawState(envs_[i], state_ptr + i * RAW_STATE_DIM);
    }
    return raw_state_cpu_.to(torch::kCUDA, /*non_blocking=*/true);
  }

  nb::tuple step(torch::Tensor actions) {
    auto actions_cpu = actions.to(torch::kCPU).contiguous();
    const f32* act_ptr = actions_cpu.data_ptr<f32>();
    f32* state_ptr = raw_state_cpu_.data_ptr<f32>();
    f32* rew_ptr = rewards_cpu_.data_ptr<f32>();
    f32* done_ptr = dones_cpu_.data_ptr<f32>();

    {
      nb::gil_scoped_release reelase;
      #pragma omp parallel for schedule(static)
      for (i32 i = 0; i < num_envs_; i++) {
        const f32* env_act =  act_ptr + i * (POD_NB * 4);
        for (i32 podId = 0; podId < POD_NB; podId++) {
          const f32* a = env_act + podId * 4;
          Pod const& pod = envs_[i].pod(podId);

          Move move{};
          f64 target_angle = pod.angle + std::clamp(static_cast<f64>(a[0]), -MAX_ROTATION, +MAX_ROTATION);

          move.target.x = std::floor(pod.x + std::cos(target_angle) * 5000 + 0.5);
          move.target.y = std::floor(pod.y + std::sin(target_angle) * 5000 + 0.5);
          move.thrust   = std::clamp(static_cast<i32>(std::lround(a[1])), 0, MAX_THRUST);
          move.shield   = (a[2] > 0.5f);
          move.boost    = (a[3] > 0.5f);

          envs_[i].applyMove(podId, move);
        }

        rew_ptr[i] = envs_[i].nextTurn();
        if (envs_[i].winnerTeam() != -1) {
          done_ptr[i] = 1.0f;
          i64 env_seed = base_seed_ + static_cast<i64>(i) * 1000000LL + (episode_counts_[i]++);
          envs_[i].initializeRefereeGenerated(LAPS, env_seed);
        } else {
          done_ptr[i] = 0.0f;
        }
        writeRawState(envs_[i], state_ptr + i * RAW_STATE_DIM);
      }
    }

    return nb::make_tuple(
      raw_state_cpu_.to(torch::kCUDA, /*non_blocking=*/true),
      rewards_cpu_.to(torch::kCUDA, /*non_blocking=*/true),
      dones_cpu_.to(torch::kCUDA, /*non_blocking=*/true)
    );
  }

private:

  static void writeRawState(Engine const& eng, f32* out) {
    auto const& pods = eng.pods();
    for (i32 p = 0; p < POD_NB; p++) {
      out[p * 8 + 0] = static_cast<f32>(pods[p].x);
      out[p * 8 + 1] = static_cast<f32>(pods[p].y);
      out[p * 8 + 2] = static_cast<f32>(pods[p].vx);
      out[p * 8 + 3] = static_cast<f32>(pods[p].vy);
      out[p * 8 + 4] = static_cast<f32>(pods[p].angle);
      out[p * 8 + 5] = static_cast<f32>(pods[p].next);
      out[p * 8 + 6] = static_cast<f32>(pods[p].shield);
      out[p * 8 + 7] = static_cast<f32>(pods[p].boosted);
    }

    auto const& cps = eng.checkpoints();
    i32 cps_len = static_cast<i32>(cps.size());
    for (i32 c = 0; c < MAX_CP; c++) {
      out[32 + c * 2 + 0] = static_cast<f32>(cps[c % cps_len].x);
      out[32 + c * 2 + 1] = static_cast<f32>(cps[c % cps_len].y);
    }
    out[44] = static_cast<f32>(cps_len);
    out[45] = static_cast<f32>(LAPS);
    out[46] = static_cast<f32>(eng.timeouts()[0]);
    out[47] = static_cast<f32>(eng.timeouts()[1]);
  }

  i32 num_envs_;
  i64 base_seed_;
  std::vector<Engine> envs_;
  std::vector<i64> episode_counts_;
  torch::Tensor raw_state_cpu_;
  torch::Tensor rewards_cpu_;
  torch::Tensor dones_cpu_;
};

NB_MODULE(engine, m) {
  nb::class_<VectorEnv>(m, "VectorEnv")
    .def(nb::init<i32, i64>())
    .def("reset", &VectorEnv::reset)
    .def("step", &VectorEnv::step);
}
