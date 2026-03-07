#include "vector.h"

#include "java_math.h"

f64 Vector::getAngle(Vector const& other) const {
  // return atan2(other.y - y, other.x - x);
  return java_math::atan2(other.y - y, other.x - x);
}

bool checkpointCollide(
  Vector const& start,
  Vector const& end,
  Checkpoint const& checkpoint) {
  f64 x2 = end.x - start.x;
  f64 y2 = end.y - start.y;
  f64 px = checkpoint.x - start.x;
  f64 py = checkpoint.y - start.y;

  f64 dotprod = px * x2 + py * y2;
  f64 projLenSq = 0.0;
  if (dotprod > 0.0) {
    px = x2 - px;
    py = y2 - py;
    dotprod = px * x2 + py * y2;
    if (dotprod > 0.0) {
      projLenSq = dotprod * dotprod / (x2 * x2 + y2 * y2);
    }
  }

  f64 lenSq = px * px + py * py - projLenSq;
  if (lenSq < 0.0) {
    lenSq = 0.0;
  }

  if (lenSq < CHECKPOINT_RADIUS_SQ) {
    return true;
  }

  return end.distanceSq(checkpoint) <= CHECKPOINT_RADIUS_SQ;
}
