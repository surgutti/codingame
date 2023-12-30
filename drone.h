#ifndef DRONE_H
#define DRONE_H

#include <string>

#include "vector.h"
#include "fish.h"

struct Drone {
    Vector pos;
    Vector speed;
    int id;
    int emergency;
    int battery;
    
    bool lights; // was light ON on the last turn?

    int scan_count;
    Fish* scans[30];
    
    std::string move;
    std::string msg;
};

#endif // DRONE_H