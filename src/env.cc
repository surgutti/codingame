#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <torch/extension.h>
#include <omp.h>

#include "sim/engine.h"

namespace nb = nanobind;

class VectorEnv {
public:
  VectorEnv(
    i32 num_envs, 
    i32 state_dim, 
    i64 seed) :
    num_envs(num_envs),
    state_dim(state_dim),
    seed(seed), envs(num_envs),
    rewards(num_envs), obs(num_envs * state_dim) {
    for (i32 i = 0; i < num_envs; i++) {
      envs[i].initializeRefereeGenerated(LAPS, seed);
      seed += 1;
    }
  }

  auto step(torch::Tensor actions) {
    auto actions_cpu = actions.to(torch::kCPU).contiguous();
    const f32* actions_ptr = actions_cpu.data_ptr<f32>();

    #pragma omp parallel for schedule(static)
    for (i32 i = 0; i < num_envs; i++) {
      // <shield, boost, thrust, angle>
      const f32* action_ptr = actions_ptr + num_envs * 4 * POD_NB;
      for (i32 podId = 0; podId < POD_NB; podId++) {
        Move move{};

        if (action_ptr[0] > 0.5) {
          move.shield = true;
        }

        if (action_ptr[1] > 0.5) {
          move.boost = true;
        }

        i32 thrust = std::lround(MAX_THRUST * action_ptr[2]);
        
        f32 angle = -MAX_ROTATION + action_ptr[3] * 2 * MAX_ROTATION;
        i32 dx = std::lround(cos(angle) * 3000);
        i32 dy = std::lround(sin(angle) * 3000);

        Pod const& pod = envs[i].pod(podId);

        move.thrust = std::clamp(thrust, 0, MAX_THRUST);
        move.target.x = pod.x + dx;
        move.target.y = pod.y + dy;

        envs[i].applyMove(podId, move);
        
        action_ptr += 4;
      }
      
      rewards[i] = envs[i].nextTurn();

      if (envs[i].winnerTeam != -1) {
        envs[i].initializeRefereeGenerated(LAPS, seed);
        seed += 1;
      }
    }

    return nb::make_tuple(
      torch::from_blob(
        rewards.data(),
        { num_envs, 1 },
        torch::TensorOptions.dtype(torch::kFloat32).device(torch::kCUDA)
      ),
      torch::from_blob(
        obs.data(),
        { num_envs, state_dim },
        torch::TensorOptions.dtype(torch::kFloat32).device(torch::kCUDA)
      )
    );
  }

private:
  i32 num_envs;
  i32 state_dim;
  i64 seed;
  std::vector<Engine> envs;
  std::vector<f32> rewards;
  std::vector<f32> obs;
};

NB_MODULE(engine, m) {
  nb::class_<VectorEnv>(m, "VectorEnv")
    .def(nb::init<i32, i32, i32>())
    .def(step, &VectorEnv::step)
}
