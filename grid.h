#ifndef GRID_H
#define GRID_H

#include <iostream>
#include <map>
#include <utility>

#include "const.h"

namespace grid {

const int GRID_SIZE = WIDTH / UNITS_PER_BLOCK;

float p[FISH_COLORS][FISH_TYPES][GRID_SIZE + 1][GRID_SIZE + 1];
float _p[GRID_SIZE + 1][GRID_SIZE + 1];

const int POSSIBLE_SIZE = 120;

int moves_grid_count;
int dx[POSSIBLE_SIZE];
int dy[POSSIBLE_SIZE];
float pp[POSSIBLE_SIZE];

int distance(int x1, int y1, int x2, int y2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

void initialize_probability() {

    const int RANGE = 200;

    std::map<std::pair<int, int>, double> S;
    int sum = 0;
    // change that to only circle from center of block -> faster but less exact
    for (int x = - UNITS_PER_BLOCK / 2; x < + UNITS_PER_BLOCK / 2; x++) {
        for (int y = - UNITS_PER_BLOCK / 2; y < + UNITS_PER_BLOCK / 2; y++) {
            const int REP = 1e3;
            for (int angle = 0; angle < REP; angle++) {
                double alpha = angle * TAU / REP; // angle * 7 * PI / 4;

                int xx = x + (int)round(cos(alpha) * RANGE);
                int yy = y + (int)round(sin(alpha) * RANGE);
            
                S[std::make_pair(xx / UNITS_PER_BLOCK, yy / UNITS_PER_BLOCK)] += 1.0 / REP;
            }
            sum++;
        }
    }

    for (int type = 0; type < FISH_TYPES; type++) {
        for (int color = 0; color < FISH_COLORS; color++) {
            const int LX = fish_nets[fish_table[type][color]->id].LU.x / UNITS_PER_BLOCK;
            const int RX = fish_nets[fish_table[type][color]->id].RD.x / UNITS_PER_BLOCK;
            const int LY = fish_nets[fish_table[type][color]->id].LU.y / UNITS_PER_BLOCK;
            const int RY = fish_nets[fish_table[type][color]->id].RD.y / UNITS_PER_BLOCK;
            const int area = (RX - LX + 1) * (RY - LY + 1);

            // remeber that on edges there is different propability -> thing to change

            for (int x = LX; x <= RX; x++) {
                for (int y = LY; y <= RY; y++) {
                    p[type][color][x][y] += 1.0 / area;
                }
            }
        }
    }

    std::cerr << "ALL: " << S.size() << '\n';

    for (auto [xy, z] : S) {
        dx[moves_grid_count] = xy.first;
        dy[moves_grid_count] = xy.second;
        pp[moves_grid_count] = z / sum;
        moves_grid_count++;
    }
}

float evaluate_fish_movement(int fish_id) {
    int x1 = drones[0][0].pos.x;
    int y1 = drones[0][0].pos.y;
    int r1 = drones[0][0].lights ? LIGHT_SCAN_RANGE : DARK_SCAN_RANGE;

    int x2 = drones[0][1].pos.x;
    int y2 = drones[0][1].pos.y;
    int r2 = drones[0][1].lights ? LIGHT_SCAN_RANGE : DARK_SCAN_RANGE;

    float score = 0;

    const Net &net = fish_nets[fish_id];
    const Fish* fish = creatures_from_id[fish_id];

    const int LX = fish_nets[fish_id].LU.x / UNITS_PER_BLOCK;
    const int RX = fish_nets[fish_id].RD.x / UNITS_PER_BLOCK;
    const int LY = fish_nets[fish_id].LU.y / UNITS_PER_BLOCK;
    const int RY = fish_nets[fish_id].RD.y / UNITS_PER_BLOCK;

    for (int x = LX; x <= RX; x++) {
        for (int y = LY; y <= RY; y++) {
            // center of the block in (x, y)
            int xx = x * UNITS_PER_BLOCK + (UNITS_PER_BLOCK / 2);
            int yy = y * UNITS_PER_BLOCK + (UNITS_PER_BLOCK / 2);

            int dist = std::min(distance(xx, yy, x1, y1) - r1 * r1,
                                distance(xx, yy, x2, y2) - r2 * r2);

            if (dist <= 0) {
                score += p[fish->color][fish->type][x][y];
            }
            else {
                score += p[fish->color][fish->type][x][y] / (1 + dist);
            }
        }
    }

    return score;
}

void spread(int fish_id) {
    int x1 = drones[0][0].pos.x;
    int y1 = drones[0][0].pos.y;
    int r1 = drones[0][0].lights ? LIGHT_SCAN_RANGE : DARK_SCAN_RANGE;

    int x2 = drones[0][1].pos.x;
    int y2 = drones[0][1].pos.y;
    int r2 = drones[0][1].lights ? LIGHT_SCAN_RANGE : DARK_SCAN_RANGE;
    
    const Fish* fish = creatures_from_id[fish_id];

    const Net &net = fish_borders[fish->type];

    const int LX = net.LU.x / UNITS_PER_BLOCK;
    const int RX = net.RD.x / UNITS_PER_BLOCK;
    const int LY = net.LU.y / UNITS_PER_BLOCK;
    const int RY = net.RD.y / UNITS_PER_BLOCK;

    for (int x = LX; x <= RX; x++) {
        for (int y = LY; y <= RY; y++) {
            _p[x][y] = 0;
        }
    }

    for (int x = LX; x <= RX; x++) {
        for (int y = LY; y <= RY; y++) {
            // TODO: check for nearest my and foe drone and move accordingly if in range

            if (p[fish->color][fish->type][x][y] == 0)
                continue;

            for (int move = 0; move < moves_grid_count; move++) {
                int xx = x + dx[move];
                int yy = y + dy[move];

                if (xx < LX || xx > RX) {
                    xx = x - dx[move];
                }

                if (yy < LY || yy > RY) {
                    yy = y - dy[move];
                }
                
                _p[xx][yy] += pp[move] * p[fish->color][fish->type][x][y];
            }
        }
    }
    
    for (int x = LX; x <= RX; x++) {
        for (int y = LY; y <= RY; y++) {
            p[fish->color][fish->type][x][y] = _p[x][y];
        }
    }
}

void cap_to_fish_net(int fish_id) {
    const Net &net = fish_nets[fish_id];
    const Fish* fish = creatures_from_id[fish_id];

    const int LX = int(fish_borders[fish->type].LU.x / UNITS_PER_BLOCK);
    const int RX = int(fish_borders[fish->type].RD.x / UNITS_PER_BLOCK);
    const int LY = int(fish_borders[fish->type].LU.y / UNITS_PER_BLOCK);
    const int RY = int(fish_borders[fish->type].RD.y / UNITS_PER_BLOCK);

    const int lx = int(net.LU.x / UNITS_PER_BLOCK);
    const int rx = int(net.RD.x / UNITS_PER_BLOCK);
    const int ly = int(net.LU.y / UNITS_PER_BLOCK);
    const int ry = int(net.RD.y / UNITS_PER_BLOCK);

    float to_delete = 0;
    for (int x = LX; x <= RX; x++) {
        for (int y = LY; y <= RY; y++) {
            if (x < lx || x > rx || y < ly || y > ry) {
                to_delete += p[fish->color][fish->type][x][y];
                p[fish->color][fish->type][x][y] = 0;
            }
        }
    }

    if (to_delete == 0)
        return;
    
    to_delete = 1.0 / (1.0 - to_delete);

    for (int x = lx; x <= rx; x++) {
        for (int y = ly; y <= ry; y++) {
            p[fish->color][fish->type][x][y] *= to_delete;
        }
    }
}

void clear_drone_view(int fish_id) {
    int x1 = drones[0][0].pos.x;
    int y1 = drones[0][0].pos.y;
    int r1 = drones[0][0].lights ? LIGHT_SCAN_RANGE : DARK_SCAN_RANGE;

    int x2 = drones[0][1].pos.x;
    int y2 = drones[0][1].pos.y;
    int r2 = drones[0][1].lights ? LIGHT_SCAN_RANGE : DARK_SCAN_RANGE;

    float to_delete = 0;
    
    const Net &net = fish_nets[fish_id];
    const Fish* fish = creatures_from_id[fish_id];
    
    for (int x = int(net.LU.x / UNITS_PER_BLOCK); x <= int(net.RD.x / UNITS_PER_BLOCK); x++) {
        for (int y = int(net.LU.x / UNITS_PER_BLOCK); y <= int(net.RD.y / UNITS_PER_BLOCK); y++) {
            if (p[fish->color][fish->type][x][y] == 0)
                continue;

            int xx = x * UNITS_PER_BLOCK + UNITS_PER_BLOCK / 2;
            int yy = y * UNITS_PER_BLOCK + UNITS_PER_BLOCK / 2;

            if (distance(xx, yy, x1, y1) <= r1 * r1 ||
                distance(xx, yy, x2, y2) <= r2 * r2) {
                to_delete += p[fish->color][fish->type][x][y];
                p[fish->color][fish->type][x][y] = 0;
            }
        }
    }

    if (to_delete == 0)
        return;

    to_delete = 1.0 / (1.0 - to_delete);

    for (int x = int(net.LU.x / UNITS_PER_BLOCK); x <= int(net.RD.x / UNITS_PER_BLOCK); x++) {
        for (int y = int(net.LU.x / UNITS_PER_BLOCK); y <= int(net.RD.y / UNITS_PER_BLOCK); y++) {
            p[fish->color][fish->type][x][y] *= to_delete;
        }
    }
}

void apply_enemy_view(int fish_id, int drone_id) {
    int drone_x = drones[1][drone_id].pos.x;
    int drone_y = drones[1][drone_id].pos.y;

    const Net &net = fish_nets[fish_id];

    const int LX = net.LU.x / UNITS_PER_BLOCK;
    const int RX = net.RD.x / UNITS_PER_BLOCK;
    const int LY = net.LU.y / UNITS_PER_BLOCK;
    const int RY = net.RD.y / UNITS_PER_BLOCK;

    for (int x = LX; x <= RX; x++) {
        for (int y = LY; y <= RY; y++) {
               
        }
    }
}

}

#endif // GRID_H