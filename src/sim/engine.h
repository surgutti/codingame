#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <array>

#include "pod.h"

struct Move {
  f64 angle;
  i32 thrust;
  bool boost;
};

class Engine {

public:

  void checkpointCompleted(int podId) {
    if (pods[podId].next == static_cast<int>(cps.size())) {
      winner |= podId;
    }
    else {
      pods[podId].next++;
    }

    timeouts[podId >= 2] = TIMEOUT;
  }

  void applyMove(int, Move const&);
  void nextTurn();

private:
  std::vector<Checkpoint> cps;
  std::array<Pod, POD_NB> pods;
  std::array<i32, PLAYER_NB> timeouts;
  u32 winner;

};

#endif // ENGINE_H