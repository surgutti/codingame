#ifndef VECTOR_H
#define VECTOR_H

#include "const.h"

#include <cmath>

struct Vector {
  f64 x = 0.0;
  f64 y = 0.0;

  f64 distanceSq(Vector const& other) const {
    f64 dx = x - other.x;
    f64 dy = y - other.y;
    return dx * dx + dy * dy;
  }

  f64 distance(Vector const& other) const {
    return std::sqrt(distanceSq(other));
  }

  f64 dot(Vector const& other) const {
    return x * other.x + y * other.y;
  }

  f64 getAngle(Vector const& other) const;

  bool operator==(Vector const& other) const {
    return x == other.x && y == other.y;
  }
};

using Checkpoint = Vector;

bool checkpointCollide(Vector const&, Vector const&, Checkpoint const&);

#endif // VECTOR_H
