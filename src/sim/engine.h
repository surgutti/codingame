#ifndef ENGINE_H
#define ENGINE_H

#include <array>
#include <utility>
#include <vector>

#include "pod.h"

class Engine {
public:
  Engine();

  void initialize(i32 laps, std::vector<std::pair<i32, i32>> const& checkpoints);
  void resetRace();
  void setTurn(i32 turn);

  void setPodState(
    i32 podId,
    i32 x,
    i32 y,
    i32 vx,
    i32 vy,
    f64 angleRad,
    i32 nextCheckpointId,
    i32 shield = 0,
    i32 boosted = 0,
    i32 progress = -1);

  Pod const& pod(i32 podId) const {
    return pods_[podId];
  }

  std::array<Pod, POD_NB> const& pods() const {
    return pods_;
  }

  std::vector<Checkpoint> const& checkpoints() const {
    return track_;
  }

  std::array<i32, PLAYER_NB> const& timeouts() const {
    return timeouts_;
  }

  i32 winnerTeam() const {
    return winnerTeam_;
  }

  void applyMove(i32 podId, Move const& move);
  void applyMoves(std::array<Move, POD_NB> const& moves);
  void nextTurn();

private:
  void applyCommand(i32 podId, Move const& move);
  void checkpointCompleted(i32 podId);

  i32 laps_ = 0;
  i32 turn_ = 0;
  i32 winnerTeam_ = -1;
  std::vector<Checkpoint> track_;
  std::array<Pod, POD_NB> pods_{};
  std::array<Move, POD_NB> queuedMoves_{};
  std::array<i32, PLAYER_NB> timeouts_{};
};

#endif // ENGINE_H
