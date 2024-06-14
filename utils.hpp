#ifndef UTILS_HPP
#define UTILS_HPP

#include <cassert>

int to_move_index(char c) {
    if (c == 'U') {
        return 0;
    }
    if (c == 'L') {
        return 1;
    }
    if (c == 'D') {
        return 2;
    }
    if (c == 'R') {
        return 3;
    }

    assert(false);
}

#endif // UTILS_HPP