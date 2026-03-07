#include "pod.h"

#include <cmath>

void bounce(Pod& a, Pod& b) {
  Vector normal{b.x - a.x, b.y - a.y};
  f64 distance = std::sqrt(normal.x * normal.x + normal.y * normal.y);
  if (distance <= EPSILON) {
    normal = {1.0, 0.0};
    distance = 1.0;
  } else {
    normal.x /= distance;
    normal.y /= distance;
  }

  Vector relativeVel{a.vx - b.vx, a.vy - b.vy};
  f64 invMassA = (a.shield == SHIELD_START) ? 0.1 : 1.0;
  f64 invMassB = (b.shield == SHIELD_START) ? 0.1 : 1.0;

  f64 force = normal.dot(relativeVel) / (invMassA + invMassB);
  if (force < 120.0) {
    force += 120.0;
  } else {
    force += force;
  }

  Vector impulse{normal.x * -force, normal.y * -force};
  a.vx += impulse.x * invMassA;
  a.vy += impulse.y * invMassA;
  b.vx -= impulse.x * invMassB;
  b.vy -= impulse.y * invMassB;

  if (distance <= POD_DIAMETER) {
    distance -= POD_DIAMETER;
    a.x += normal.x * -(-distance / 2.0 + EPSILON);
    a.y += normal.y * -(-distance / 2.0 + EPSILON);
    b.x += normal.x * (+(-distance / 2.0 + EPSILON));
    b.y += normal.y * (+(-distance / 2.0 + EPSILON));
  }
}
