#ifndef GAME_H
#define GAME_H

#include <vector>
#include <array>

#include "pod.h"

struct Move {
  f64 angle;
  i32 thrust;
  bool boost;
};

class Game {

public:

private:
  std::vector<Checkpoint> cps;
  std::array<Pod, 4> pods;

};

#endif // GAME_H