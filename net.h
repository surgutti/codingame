#ifndef NET_H
#define NET_H

#include <algorithm>
#include <cassert>

#include "vector.h"

struct Net {
    Vector LU; // left up
    Vector RD; // right down

    Net() {

    }

    Net(const Vector &_LU, const Vector &_RD) :
        LU(_LU), RD(_RD) {
        assert(LU.x <= RD.x);
        assert(LU.y <= RD.y);
    }

    bool inside(const Vector &point) const {
        return LU.x <= point.x && point.x <= RD.x &&
               LU.y <= point.y && point.y <= RD.y;
    }

    Net intersect(const Net &other) const {
        Net result;

        result.LU.x = std::max(LU.x, other.LU.x);
        result.LU.y = std::max(LU.y, other.LU.y);

        result.RD.x = std::min(RD.x, other.RD.x);
        result.RD.y = std::min(RD.y, other.RD.y);

        assert(result.LU.x <= result.RD.x);
        assert(result.LU.y <= result.RD.y);

        return result;
    }
    
    void apply_intersection(const Net &other) {
        (*this) = this->intersect(other);
    }

    Vector center() const {
        return Vector((LU.x + RD.x) * 0.5, (LU.y + RD.y) * 0.5);
    }

    void expand(double range) {
        LU.x -= range;
        LU.y -= range;
        RD.x += range;
        RD.y += range;
    }

    bool inRange(const Vector &point, double range) const {
        return LU.x - range <= point.x && point.x <= RD.x + range &&
               LU.y - range <= point.y && point.y <= RD.y + range;
    }

    double getArea() const {
        return (RD.x - LU.x + 1) * (RD.y - LU.y + 1);
    }
};

#endif // NET_H