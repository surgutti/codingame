#ifndef POD_H
#define POD_H

#include "unit.h"

struct Pod : Unit {
  f64 angle;
  
  int next;
  int shield;
  int boosted;

  void applyThrust(int thrust) {
    f64 cs = sin(angle);
    f64 cc = cos(angle);
    vx += cc * thrust;
    vy += cs * thrust;
  }

  f64 diffAngle(Vector const& v) const {
    f64 a = getAngle(v);
    f64 da = fmod(a - angle, 2 * PI);
    return fmod(2 * da, 2 * PI) - da;
  }

  void applyRotate(Vector const& p) {
    f64 a = getAngle(p);
    f64 d = diffAngle(p);

    if (d < -MAX_ROTATION) {
      a = angle - MAX_ROTATION;
    }
    else if (d > +MAX_ROTATION) {
      a = angle + MAX_ROTATION;
    }

    angle = a;
  }
};

void bounce(Pod*, Pod*);

#endif // POD_H