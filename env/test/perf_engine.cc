#include <iostream>
#include <iomanip>
#include <utility>
#include <vector>
#include <chrono>

#include "../sim/engine.h"

i64 seed = 2137;

int main() {
  Engine engine;
  engine.initializeRefereeGenerated(2, seed);

  const i64 iterations = 100'000'000;

  auto start = std::chrono::steady_clock::now();

  i64 resets = 0;
  for (i64 iter = 0; iter < iterations; iter++) {
  
    auto checkpoints = engine.checkpoints();
    for (i32 podId = 0; podId < POD_NB; podId++) {
      Pod const& pod = engine.pod(podId);
  
      Move move {
        checkpoints[pod.next % (i32) checkpoints.size()],
        30,
        false,
        false
      };

      engine.applyMove(podId, move);
    }

    engine.nextTurn();

    if (engine.winnerTeam() != -1) {
      engine.resetRace();
      resets++;
    }
  }
  auto end = std::chrono::steady_clock::now();

  std::chrono::duration<long double, std::milli> elapsed = end - start;
  auto elapsed_sec = std::chrono::duration<long double>(end - start);

  std::cout << std::fixed << std::setprecision(3);
  std::cout << "Elapsed time: " << elapsed.count() << " ms\n";
  std::cout << "Throughput: " << iterations / elapsed_sec.count() << " iter/s\n";
  std::cout << "Time per iteration: " << (elapsed.count() * 1e6) / iterations << " ns\n";
  std::cout << "Avg. game length: " << (long double) iterations / resets << " iter\n";

  return 0;
}
