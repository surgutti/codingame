#include "pod.h"

void bounce(Pod* a, Pod* b) {
  Vector n {b->x-a->x, b->y-a->y};
  f64 dd = sqrt(n.x*n.x+n.y*n.y);
  n.x /= dd;
  n.y /= dd;

  Vector r {a->vx-b->vx, a->vy-b->vy};
  f64 m1 = (a->shield == SHIELD_START ? 0.1 : 1);
  f64 m2 = (b->shield == SHIELD_START ? 0.1 : 1);
  f64 mcoef = (m1+m2)/(m1*m2);

  f64 force = (n.x*r.x+n.y*r.y)/(m1+m2);
  if (force < 120) {
    force += 120;
  }
  else {
    force += force;
  }

  Vector i {n.x*(-force), n.y*(-force)};

  a->vx += i.x*m1;
  a->vy += i.y*m1;
  b->vx -= i.x*m2;
  b->vy -= i.y*m2;

  if (dd <= 2 * POD_RADIUS) {
    dd -= 2 * POD_RADIUS;
    a->x += (n.x * -(-dd/2 + EPSILON));
    a->y += (n.y * -(-dd/2 + EPSILON));
    b->x += (n.x * +(-dd/2 + EPSILON));
    b->y += (n.y * +(-dd/2 + EPSILON));
  }
}