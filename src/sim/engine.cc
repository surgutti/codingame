#include "engine.h"

#include <algorithm>
#include <cmath>

namespace {

struct CollisionEvent {
  f64 time = 0.0;
  i32 a = -1;
  i32 b = -1;
};

constexpr std::array<Vector, POD_NB> START_OFFSETS = {
  Vector{500.0, -500.0},
  Vector{-500.0, 500.0},
  Vector{1500.0, -1500.0},
  Vector{-1500.0, 1500.0},
};

f64 roundHalfUp(f64 value) {
  return std::floor(value + 0.5);
}

bool pathHitsCheckpoint(
  Vector const& start,
  std::vector<Vector> const& waypoints,
  Vector const& end,
  Checkpoint const& checkpoint) {
  Vector previous = start;
  for (Vector const& waypoint : waypoints) {
    if (checkpointCollide(previous, waypoint, checkpoint)) {
      return true;
    }
    previous = waypoint;
  }

  return checkpointCollide(previous, end, checkpoint);
}

} // namespace

Engine::Engine() {
  timeouts_.fill(TIMEOUT);
}

void Engine::initialize(i32 laps, std::vector<std::pair<i32, i32>> const& checkpoints) {
  laps_ = laps;
  track_.clear();
  track_.reserve(checkpoints.size());
  for (auto const& checkpoint : checkpoints) {
    track_.push_back(Checkpoint{
      static_cast<f64>(checkpoint.first),
      static_cast<f64>(checkpoint.second),
    });
  }
  resetRace();
}

void Engine::resetRace() {
  turn_ = 0;
  winnerTeam_ = -1;
  timeouts_.fill(TIMEOUT);
  queuedMoves_.fill(Move{});
  pods_.fill(Pod{});

  if (track_.size() < 2) {
    return;
  }

  Vector direction{
    track_[1].x - track_[0].x,
    track_[1].y - track_[0].y,
  };
  f64 norm = std::sqrt(direction.x * direction.x + direction.y * direction.y);
  if (norm <= 0.0) {
    norm = 1.0;
  }
  direction.x /= norm;
  direction.y /= norm;

  for (i32 podId = 0; podId < POD_NB; ++podId) {
    Pod& pod = pods_[podId];
    pod.angle = START_ANGLE;
    pod.next = 1;
    pod.x = roundHalfUp(track_[0].x + direction.y * START_OFFSETS[podId].x);
    pod.y = roundHalfUp(track_[0].y + direction.x * START_OFFSETS[podId].y);
  }
}

void Engine::setTurn(i32 turn) {
  turn_ = turn;
}

void Engine::setPodState(
  i32 podId,
  i32 x,
  i32 y,
  i32 vx,
  i32 vy,
  f64 angleRad,
  i32 nextCheckpointId,
  i32 shield,
  i32 boosted,
  i32 progress) {
  Pod& pod = pods_[podId];
  pod.x = static_cast<f64>(x);
  pod.y = static_cast<f64>(y);
  pod.vx = static_cast<f64>(vx);
  pod.vy = static_cast<f64>(vy);
  pod.angle = angleRad;
  pod.next = (progress >= 0) ? progress : nextCheckpointId;
  pod.shield = shield;
  pod.boosted = boosted;
  pod.won = false;
}

void Engine::applyMove(i32 podId, Move const& move) {
  queuedMoves_[podId] = move;
}

void Engine::applyMoves(std::array<Move, POD_NB> const& moves) {
  queuedMoves_ = moves;
}

void Engine::applyCommand(i32 podId, Move const& move) {
  Pod& pod = pods_[podId];

  i32 thrust = std::clamp(move.thrust, 0, MAX_THRUST);
  if (move.boost) {
    if (pod.boosted == 0) {
      pod.boosted = 1;
      thrust = BOOST_THRUST;
    } else {
      thrust = MAX_THRUST;
    }
  }

  if (move.shield) {
    pod.shield = SHIELD_START;
  }

  if (pod.shield > 0) {
    thrust = 0;
  }

  if (move.target == Vector{pod.x, pod.y}) {
    return;
  }

  pod.applyRotate(move.target, turn_ == 0);
  pod.applyThrust(thrust);
}

void Engine::checkpointCompleted(i32 podId) {
  Pod& pod = pods_[podId];
  ++pod.next;
  timeouts_[podId / PODS_PER_PLAYER] = TIMEOUT;

  i32 finishIndex = laps_ * static_cast<i32>(track_.size());
  if (!track_.empty() && pod.next >= finishIndex + 1) {
    pod.next = finishIndex;
    pod.won = true;
    winnerTeam_ = podId / PODS_PER_PLAYER;
  }
}

void Engine::nextTurn() {
  if (track_.empty()) {
    return;
  }

  std::array<Vector, POD_NB> startPositions{};
  for (i32 podId = 0; podId < POD_NB; ++podId) {
    startPositions[podId] = Vector{pods_[podId].x, pods_[podId].y};
    applyCommand(podId, queuedMoves_[podId]);
  }

  std::array<std::vector<Vector>, POD_NB> waypoints{};
  f64 time = 0.0;
  while (time < 1.0) {
    std::vector<CollisionEvent> collisions;
    f64 minTime = 1.0 - time;

    for (i32 i = POD_NB - 1; i > 0; --i) {
      for (i32 j = i - 1; j >= 0; --j) {
        if (pods_[j].distanceSq(pods_[i]) <= POD_DIAMETER_SQ) {
          collisions.push_back(CollisionEvent{time, i, j});
          continue;
        }

        f64 collisionTime = pods_[i].collisionTime(pods_[j], POD_DIAMETER_SQ);
        if (collisionTime <= minTime && collisionTime > 0.0) {
          collisions.push_back(CollisionEvent{time + collisionTime, i, j});
        }
      }
    }

    if (collisions.empty()) {
      for (Pod& pod : pods_) {
        pod.move(1.0 - time);
      }
      break;
    }

    std::stable_sort(
      collisions.begin(),
      collisions.end(),
      [](CollisionEvent const& left, CollisionEvent const& right) {
        return left.time < right.time;
      });

    f64 collisionTime = collisions.front().time;
    for (Pod& pod : pods_) {
      pod.move(collisionTime - time);
    }
    time = collisionTime;

    std::array<bool, POD_NB> touched{};
    for (CollisionEvent const& collision : collisions) {
      if (collision.time > collisionTime + EPSILON) {
        break;
      }

      bounce(pods_[collision.a], pods_[collision.b]);
      touched[collision.a] = true;
      touched[collision.b] = true;
    }

    for (i32 podId = 0; podId < POD_NB; ++podId) {
      if (touched[podId]) {
        waypoints[podId].push_back(Vector{pods_[podId].x, pods_[podId].y});
      }
    }
  }

  for (Pod& pod : pods_) {
    pod.endTurn();
  }

  i32 trackSize = static_cast<i32>(track_.size());
  for (i32 podId = 0; podId < POD_NB; ++podId) {
    Vector finalPosition{pods_[podId].x, pods_[podId].y};
    while (!pods_[podId].won &&
           pathHitsCheckpoint(
             startPositions[podId],
             waypoints[podId],
             finalPosition,
             track_[pods_[podId].next % trackSize])) {
      checkpointCompleted(podId);
    }
  }

  for (i32 player = 0; player < PLAYER_NB; ++player) {
    --timeouts_[player];
  }

  queuedMoves_.fill(Move{});
  ++turn_;
}
