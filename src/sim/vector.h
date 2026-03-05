#ifndef VECTOR_H
#define VECTOR_H

#include "const.h"

#include <cmath>

struct Vector {
  f64 x, y;

  f64 distance(Vector const& p) const {
    return sqrt((x-p.x)*(x-p.x)+(y-p.y)*(y-p.y));
  }
  
  f64 getAngle(Vector const& v) const {
    return atan2(v.x - x, v.y - y);
  }

  void move(f64 x_, f64 y_) {
    x = x_;
    y = y_;
  }

  void moveTo(Vector const& p, f64 dist) {
    f64 d = distance(p);
    if (d < EPSILON) {
      return;
    }

    f64 dx = p.x - x;
    f64 dy = p.y - y;
    f64 coef = dist / d;

    x += dx * coef;
    y += dy * coef;
  }

  Vector getPoint(Vector const& target, f64 dist) const {
    f64 d = distance(target);

    if (d < EPSILON) {
      return target;
    }

    f64 dx = target.x - x;
    f64 dy = target.y - y;
    f64 coef = dist / d;

    f64 x_ = x + dx * coef;
    f64 y_ = y + dy * coef;

    return Vector{x_, y_};
  }

  bool isInRange(Vector const& p, f64 range) const {
    return distance(p) <= range;
  }

};

using Checkpoint = Vector;

bool checkpointCollide(Vector const&, Vector const&, Checkpoint const&);

#endif // VECTOR_H