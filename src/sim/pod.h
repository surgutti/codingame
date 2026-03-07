#ifndef POD_H
#define POD_H

#include "unit.h"

struct Move {
  Vector target{};
  i32 thrust = 0;
  bool shield = false;
  bool boost = false;
};

struct Pod : Unit {
  f64 angle = 0.0;
  i32 next = 1;
  i32 shield = 0;
  i32 boosted = 0;
  bool won = false;

  void applyThrust(i32 thrustPower) {
    vx += std::cos(angle) * thrustPower;
    vy += std::sin(angle) * thrustPower;
  }

  f64 diffAngle(Vector const& target) const {
    f64 targetAngle = getAngle(target);
    f64 delta = std::fmod(targetAngle - angle, 2.0 * PI);
    return std::fmod(2.0 * delta, 2.0 * PI) - delta;
  }

  void applyRotate(Vector const& target, bool firstTurn) {
    if (firstTurn) {
      angle = 0.0;
      angle = diffAngle(target);
      return;
    }

    f64 targetAngle = getAngle(target);
    f64 delta = diffAngle(target);
    if (delta <= -MAX_ROTATION) {
      targetAngle = angle - MAX_ROTATION;
    } else if (delta >= MAX_ROTATION) {
      targetAngle = angle + MAX_ROTATION;
    }
    angle = targetAngle;
  }

  void endTurn() {
    adjust();
    if (shield > 0) {
      --shield;
    }
  }
};

void bounce(Pod&, Pod&);

#endif // POD_H
