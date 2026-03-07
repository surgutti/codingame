#ifndef ENGINE_H
#define ENGINE_H

#include <array>
#include <utility>
#include <vector>

#include "pod.h"

struct CollisionDebug {
  i32 id = -1;
  f64 time = 0.0;
  i32 a = -1;
  i32 ax = 0;
  i32 ay = 0;
  i32 b = -1;
  i32 bx = 0;
  i32 by = 0;
  f64 force = 0.0;
  i32 vx = 0;
  i32 vy = 0;
};

class Engine {
public:
  Engine();

  void initialize(i32 laps, std::vector<std::pair<i32, i32>> const& checkpoints, i32 podTimeout = TIMEOUT);
  void initializeRefereeGenerated(i32 laps, i64 seed, i32 mapIndex = -1, i32 podTimeout = TIMEOUT);
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

  std::vector<CollisionDebug> const& collisions() const {
    return collisions_;
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
  i32 podTimeout_ = TIMEOUT;
  i32 turn_ = 0;
  i32 winnerTeam_ = -1;
  i32 nextCollisionId_ = 0;
  std::vector<Checkpoint> track_;
  std::vector<CollisionDebug> collisions_;
  std::array<Pod, POD_NB> pods_{};
  std::array<Move, POD_NB> queuedMoves_{};
  std::array<i32, PLAYER_NB> timeouts_{};
};

#endif // ENGINE_H
