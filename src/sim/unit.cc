#include "unit.h"

#include <cmath>

f64 Unit::collisionTime(Unit const& other, f64 radiusSq) const {
  Vector relativePos{other.x - x, other.y - y};
  f64 posLenSq = relativePos.dot(relativePos);
  if (posLenSq <= radiusSq) {
    return 0.0;
  }

  Vector relativeVel{other.vx - vx, other.vy - vy};
  f64 approach = relativePos.dot(relativeVel);
  if (approach > 0.0) {
    return NO_COLLISION;
  }

  f64 velLenSq = relativeVel.dot(relativeVel);
  if (velLenSq <= 0.0) {
    return NO_COLLISION;
  }

  f64 disc = approach * approach - velLenSq * (posLenSq - radiusSq);
  if (disc <= 0.0) {
    return NO_COLLISION;
  }

  return (-approach - std::sqrt(disc)) / velLenSq;
}
