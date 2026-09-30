#ifndef MAPGEN_H
#define MAPGEN_H

#include <utility>
#include <vector>

#include "const.h"

std::vector<std::pair<i32, i32>> const& refereeBaseMap(i32 mapIndex);
std::vector<std::pair<i32, i32>> generateRefereeMap(i64 seed, i32 mapIndex = -1);

#endif // MAPGEN_H
