#include "unit.h"

f64 Unit::getCollision(Unit const& u, f64 checkedRadius) const {
  if (distance(u) <= checkedRadius) {
    return 0.0;
  }

  if (vx == 0.0 && vy == 0.0 && u.vx == 0.0 && u.vy == 0.0) {
    return NULL_COLLISION;
  }

  f64 x2 = x - u.x;
  f64 y2 = y - u.y;
  f64 r2 = checkedRadius;
  f64 vx2 = vx - u.vx;
  f64 vy2 = vy - u.vy;

  f64 a = vx2 * vx2 + vy2 * vy2;

  if (a <= 0.0) {
      return NULL_COLLISION;
  }

  f64 b = 2.0 * (x2 * vx2 + y2 * vy2);
  f64 c = x2 * x2 + y2 * y2 - r2 * r2;
  f64 delta = b * b - 4.0 * a * c;

  if (delta < 0.0) {
      return NULL_COLLISION;
  }

  f64 t = (-b - sqrt(delta)) / (2.0 * a);

  if (t <= 0.0) {
      return NULL_COLLISION;
  }

  return t;
}