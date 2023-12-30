#ifndef FISH_H
#define FISH_H

#include "vector.h"

struct Fish {
    Vector pos;
    Vector speed;
    int id;
    int type;
    int color;
    bool is_visible;
    bool is_scanned;
    bool is_foe_scanned;
    bool in_gamezone;
    bool is_reported;
    bool is_foe_reported;

    bool is_frightened;
    bool has_left;

    bool isMonster() const {
        return type == -1;
    }
};

#endif // FISH_H