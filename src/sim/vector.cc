#include "vector.h"

bool checkpointCollide(
  Vector const& p1, 
  Vector const& p2, 
  Checkpoint const& cp) {
  f64 dx = p2.x-p1.x;
  f64 dy = p2.y-p1.y;
  f64 dd = dx*dx+dy*dy;
  
  Vector pp = p1;
  if (dd != 0) {
    f64 u = ((cp.x-p1.x)*dx+(cp.y-p1.y)*dy)/dd;
    if (u > 1) {
      pp = p2;
    }
    else if (u > 0) {
      pp.x = p1.x+u*dx;
      pp.y = p1.y+u*dy;
    }
  }

  return pp.distance(cp) < CHECKPOINT_RADIUS;
}