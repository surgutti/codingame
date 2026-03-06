#ifndef UNIT_H
#define UNIT_H

#include "vector.h"

#include <cmath>

struct Unit : Vector {
  f64 vx, vy;
  f64 mass = 1.0;
  f64 friction;

  Unit(f64 x_, f64 y_) : Vector(x_,y_) { }

  f64 getSpeed() const {
    return sqrt(vx*vx+vy*vy);
  }

  f64 getSpeedAngle() const {
    return atan2(vy, vx);
  }

  void move(f64 t) {
    x += vx * t;
    y += vy * t;
  }

  void adjust() {
    x = floor(x + 0.5);
    y = floor(y + 0.5);

    vx = trunc(vx * (1.0 - friction));
    vy = trunc(vy * (1.0 - friction));
  }

  f64 getCollision(Unit const&, double) const;
};

#endif // UNIT_H