#ifndef POD_H
#define POD_H

#include "unit.h"

using Checkpoint = Vector;

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

  f64 diffAngle(Unit const& u) const {
    f64 a = getAngle(u);
    f64 da = fmod(a - angle, 2 * PI);
    return fmod(2 * da, 2 * PI) - da;
  }
};

void bounce(Pod*, Pod*);

#endif // POD_H