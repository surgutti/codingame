#ifndef COLLISION_H
#define COLLISION_H

#include <cmath>

#include "const.h"
#include "vector.h"
#include "drone.h"
#include "fish.h"

double getCollision(Drone drone, Fish ugly) {
    // Check instant collision
    if (ugly.pos.inRange(drone.pos, DRONE_HIT_RANGE + UGLY_EAT_RANGE)) {
        return 0.0;
    }

    // Both units are motionless
    if (drone.speed.isZero() && ugly.speed.isZero()) {
        return -1.0;
    }

    // Change referencial
    double x = ugly.pos.x;
    double y = ugly.pos.y;
    double ux = drone.pos.x;
    double uy = drone.pos.y;

    double x2 = x - ux;
    double y2 = y - uy;
    double r2 = UGLY_EAT_RANGE + DRONE_HIT_RANGE;
    double vx2 = ugly.speed.x - drone.speed.x;
    double vy2 = ugly.speed.y - drone.speed.y;

    // Resolving: sqrt((x + t*vx)^2 + (y + t*vy)^2) = radius <=> t^2*(vx^2 + vy^2) + t*2*(x*vx + y*vy) + x^2 + y^2 - radius^2 = 0
    // at^2 + bt + c = 0;
    // a = vx^2 + vy^2
    // b = 2*(x*vx + y*vy)
    // c = x^2 + y^2 - radius^2 

    double a = vx2 * vx2 + vy2 * vy2;

    if (a <= 0.0) {
        return -1.0;
    }

    double b = 2.0 * (x2 * vx2 + y2 * vy2);
    double c = x2 * x2 + y2 * y2 - r2 * r2;
    double delta = b * b - 4.0 * a * c;

    if (delta < 0.0) {
        return -1.0;
    }

    double t = (-b - sqrt(delta)) / (2.0 * a);

    if (t <= 0.0) {
        return -1.0;
    }

    if (t > 1.0) {
        return -1.0;
    }
    return t;
}


#endif // COLLISION_H