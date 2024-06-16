#ifndef ROLLER_SKATING
#define ROLLER_SKATING

#include "../random.hpp"

#include <iostream>
#include <algorithm>

constexpr uint8_t all_permutations[24] = {
    27,
    30,
    39,
    45,
    54,
    57,
    75,
    78,
    99,
    108,
    114,
    120,
    135,
    141,
    147,
    156,
    177,
    180,
    198,
    201,
    210,
    216,
    225,
    228,
};

// constexpr int8_t all_permutations[24][4] = {
//     {0, 1, 2, 3},
//     {0, 1, 3, 2},
//     {0, 2, 1, 3},
//     {0, 2, 3, 1},
//     {0, 3, 1, 2},
//     {0, 3, 2, 1},
//     {1, 0, 2, 3},
//     {1, 0, 3, 2},
//     {1, 2, 0, 3},
//     {1, 2, 3, 0},
//     {1, 3, 0, 2},
//     {1, 3, 2, 0},
//     {2, 0, 1, 3},
//     {2, 0, 3, 1},
//     {2, 1, 0, 3},
//     {2, 1, 3, 0},
//     {2, 3, 0, 1},
//     {2, 3, 1, 0},
//     {3, 0, 1, 2},
//     {3, 0, 2, 1},
//     {3, 1, 0, 2},
//     {3, 1, 2, 0},
//     {3, 2, 0, 1},
//     {3, 2, 1, 0}
// };

// (order >> move[i]) & 3 == 0

struct RollerSkating {

    int8_t turns_left;
    // int8_t turns_done;

    int8_t dist[3];
    int8_t risk[3];

    uint8_t order;

    bool end;

    void debug() const {
        std::cerr << "TURNS LEFT: " << int(turns_left) << '\n';
        for (int i = 0; i < 3; i++) {
            std::cerr << "i: " << i << ' ' << int(dist[i]) << ' ' << int(risk[i]) << '\n';
        }
        std::cerr << "order: ";
        for (int i = 0; i < 4; i++) {
            std::cerr << int((order >> (i << 1)) & 3) << ' ';
        }
        std::cerr << '\n';
    }

    void generate_places(int8_t* places) {
        for (int i = 0; i < 3; i++) {
            if (dist[i] >= dist[(i + 1) % 3] && dist[i] >= dist[(i + 2) % 3]) {
                places[i] = 3;
            }
            else
            if (dist[i] < dist[(i + 1) % 3] && dist[i] < dist[(i + 2) % 3]) {
                places[i] = 0;
            }
            else {
                places[i] = 1;
            }
        }
    }

    // TODO:
    // after few turns (5?) just take the places as dist shows
    // gather some statistics on starting positions
    // the most important fact is that players will jump on max and get higest risk
    inline void play(const int8_t* move) {
        if (end) {
            return;
        }

        for (int i = 0; i < 3; i++) {
            if (risk[i] < 0) {
                risk[i]++;
            }
            else {
                const int8_t index = (order >> (move[i] << 1)) & 3;
                
                dist[i] += 2;

                if (index == 0) {
                    dist[i]--;
                    risk[i]--;
                }
                else
                if (index == 2) {
                    risk[i]++;
                }
                else
                if (index == 3) {
                    dist[i]++;
                    risk[i] += 2;
                }
            }
        }

        if (risk[0] >= 0 &&
            (dist[0] % 10 == dist[1] % 10 ||
            dist[0] % 10 == dist[2] % 10)) {
            risk[0] += 2;
        }

        if (risk[1] >= 0 &&
            (dist[1] % 10 == dist[0] % 10 ||
            dist[1] % 10 == dist[2] % 10)) {
            risk[1] += 2;
        }

        if (risk[2] >= 0 &&
            (dist[2] % 10 == dist[0] % 10 ||
            dist[2] % 10 == dist[1] % 10)) {
            risk[2] += 2;
        }

        for (int i = 0; i < 3; i++) {
            if (risk[i] >= 5) {
                risk[i] = -2;
            }
        }

        if (turns_left == 0 /*|| turns_done == 10*/) {
            end = true;
        }
        else {
            // turns_done++;
            turns_left--;

            order = all_permutations[fast_rand() % 24];
        }
    }

    bool in_waiting(const int8_t player_idx) const {
        return risk[player_idx] < 0;
    }

    // maybe if for not being stun'ed
    int8_t greedy_move(const int8_t player_idx) const {
        if (risk[player_idx] + 2 < 5) {
            return (order >> (2 * 3)) & 3; // if have risk + 2 < 5 then rush 3
        }
        return (order >> 1) & 3; // else go 2
    }
};

#endif // ROLLER_SPEED_SKATING