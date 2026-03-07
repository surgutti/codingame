#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "../sim/engine.h"

namespace {

Move parseMove(std::string const& line) {
  std::istringstream stream(line);
  Move move;
  std::string thrustToken;
  stream >> move.target.x >> move.target.y >> thrustToken;
  if (thrustToken == "SHIELD") {
    move.shield = true;
  } else if (thrustToken == "BOOST") {
    move.boost = true;
  } else {
    move.thrust = std::stoi(thrustToken);
  }
  return move;
}

} // namespace

int main(int argc, char** argv) {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  bool continuous = false;
  bool preserveAngle = false;
  for (int argi = 1; argi < argc; ++argi) {
    if (std::string_view(argv[argi]) == "--continuous") {
      continuous = true;
    } else if (std::string_view(argv[argi]) == "--preserve-angle") {
      preserveAngle = true;
    }
  }

  i32 checkpointCount = 0;
  if (!(std::cin >> checkpointCount)) {
    return 0;
  }

  std::vector<std::pair<i32, i32>> checkpoints;
  checkpoints.reserve(checkpointCount);
  for (i32 i = 0; i < checkpointCount; ++i) {
    i32 x = 0;
    i32 y = 0;
    std::cin >> x >> y;
    checkpoints.emplace_back(x, y);
  }

  i32 tests = 0;
  std::cin >> tests;
  std::string line;
  std::getline(std::cin, line);

  Engine engine;
  i32 laps = (15 + checkpointCount - 1) / checkpointCount;
  engine.initialize(laps, checkpoints);
  i32 checkpointCountValue = static_cast<i32>(checkpoints.size());

  std::cout << std::fixed << std::setprecision(15);
  for (i32 turn = 0; turn < tests; ++turn) {
    if (!continuous) {
      engine.setTurn(turn);
    }

    for (i32 podId = 0; podId < POD_NB; ++podId) {
      std::getline(std::cin, line);
      while (line.empty() && std::cin.good()) {
        std::getline(std::cin, line);
      }
      if (line.empty()) {
        return 0;
      }

      std::istringstream stream(line);
      i32 x = 0;
      i32 y = 0;
      i32 vx = 0;
      i32 vy = 0;
      f64 angleRad = 0.0;
      i32 next = 0;
      i32 shield = 0;
      i32 boosted = 0;
      i32 progress = -1;
      stream >> x >> y >> vx >> vy >> angleRad >> next >> shield >> boosted >> progress;
      if (!continuous) {
        if (preserveAngle && turn > 0) {
          angleRad = engine.pod(podId).angle;
        }
        engine.setPodState(podId, x, y, vx, vy, angleRad, next, shield, boosted, progress);
      }
    }

    std::array<Move, POD_NB> moves{};
    for (i32 podId = 0; podId < POD_NB; ++podId) {
      std::getline(std::cin, line);
      while (line.empty() && std::cin.good()) {
        std::getline(std::cin, line);
      }
      if (line.empty()) {
        return 0;
      }
      moves[podId] = parseMove(line);
    }

    engine.applyMoves(moves);
    engine.nextTurn();

    for (i32 podId = 0; podId < POD_NB; ++podId) {
      Pod const& pod = engine.pod(podId);
      std::cout
        << static_cast<i32>(pod.x) << ' '
        << static_cast<i32>(pod.y) << ' '
        << static_cast<i32>(pod.vx) << ' '
        << static_cast<i32>(pod.vy) << ' '
        << pod.angle << ' '
        << (pod.next % checkpointCountValue) << ' '
        << pod.shield << ' '
        << pod.boosted << '\n';
    }
  }

  return 0;
}
