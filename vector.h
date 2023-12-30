#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>
#include <string>

struct Vector {
    double x;
    double y;

    Vector(double _x = 0, double _y = 0) :
        x(_x), y(_y) {

    }

    Vector round() const {
        return Vector(
            (int) ::round(this->x), 
            (int) ::round(this->y)
        );
    }

    bool isZero() const {
        return x == 0 && y == 0;
    }
    
    Vector normalize() {
        double length = sqrt(x * x + y * y);
        if (length == 0)
            return Vector(0, 0);
        return Vector(x / length, y / length);
    }

    bool inRange(const Vector &v, double range) const {
        return (v.x - x) * (v.x - x) + (v.y - y) * (v.y - y) <= range * range;
    }

    double distance(const Vector &other) const {
        return sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
    }

    Vector operator+ (const Vector &other) const {
        return Vector(x + other.x, y + other.y);
    }

    Vector operator- (const Vector &other) const {
        return Vector(x - other.x, y - other.y);
    }

    Vector operator* (const double &scalar) const {
        return Vector(x * scalar, y * scalar);
    }

    std::string to_string() const {
        return std::to_string(x) + ',' + std::to_string(y);
    }
};

#endif // VECTOR_H