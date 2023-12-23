#ifndef FISH_H
#define FISH_H

#include "vector.h"

struct Fish {
    Vector pos;
    Vector speed;
    int id;
    int type;
    int color;

    bool isMonster() const {
        return type == -1;
    }
};

#endif // FISH_H