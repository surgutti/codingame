#ifndef UNIT_H
#define UNIT_H

#include "vector.h"

struct Unit : Vector {
  f64 vx = 0.0;
  f64 vy = 0.0;

  Unit() = default;
  Unit(f64 x_, f64 y_) {
    x = x_;
    y = y_;
  }

  void move(f64 t) {
    x += vx * t;
    y += vy * t;
  }

  void adjust() {
    x = std::floor(x + 0.5);
    y = std::floor(y + 0.5);
    vx = std::trunc(vx * FRICTION_FACTOR);
    vy = std::trunc(vy * FRICTION_FACTOR);
  }

  f64 collisionTime(Unit const&, f64 radiusSq) const;
};

#endif // UNIT_H
