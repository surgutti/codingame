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

  // apply move on a given pod
  void applyMove(int, Move const&);

  void nextTurn();

private:
  std::vector<Checkpoint> cps;
  std::array<Pod, POD_NB> pods;
  u32 winner;
};

#endif // ENGINE_H