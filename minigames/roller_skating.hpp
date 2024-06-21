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

// const uint8_t second_in_permutation[256] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,3,0,0,0,0,0,3,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
// const uint8_t last_in_permutation[256] =   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,3,0,0,0,0,0,0,0,0,3,0,0,0,0,0,3,0,0,0,0,0,0,0,0,3,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};

// void index_in_permtation_init() {
//     for (int i = 0; i < 24; i++) {
//         for (int j = 0; j < 4; j++) {
//             // if (((all_permutations[i] >> (2 * j)) & 3) == 1) {
//             //     second_in_permutation[all_permutations[i]] = j;
//             // }

//             // if (((all_permutations[i] >> (2 * j)) & 3) == 3) {
//             //     last_in_permutation[all_permutations[i]] = j;
//             // }
//         }
//     }

//     for (int i = 0; i < 256; i++) {
//         std::cerr << int(last_in_permutation[i]) << ',';
//     }
//     std::cerr << '\n';
// }

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

    // int8_t turns_done;

    int8_t risk[3];
    int8_t dist_div10[3];
    int8_t dist_mod10[3];
    
    int8_t turns_left;
    uint8_t order;

    bool end;

    bool operator== (const RollerSkating &other) const {
        if (end != other.end)
            return false;
        
        if (end)
            return true;

        for (int i = 0; i < 3; i++) {
            if (dist_div10[i] != other.dist_div10[i])
                return false;
            if (dist_mod10[i] != other.dist_mod10[i])
                return false;
            if (risk[i] != other.risk[i])
                return false;
        }

        // return order == other.order &&
        return turns_left == other.turns_left;
    }

    void debug() const {
        std::cerr << "TURNS LEFT: " << int(turns_left) << '\n';
        for (int i = 0; i < 3; i++) {
            std::cerr << "i: " << i << ' ' << int(dist_div10[i] * 10 + dist_mod10[i]) << ' ' << int(risk[i]) << '\n';
        }
        std::cerr << "order: ";
        for (int i = 0; i < 4; i++) {
            std::cerr << int((order >> (i << 1)) & 3) << ' ';
        }
        std::cerr << '\n';
    }

    inline int expected_end() const {
        return turns_left;
    }

    inline void generate_places(int8_t* places) {
        const int8_t d0 = dist_div10[0] * 10 + dist_mod10[0];
        const int8_t d1 = dist_div10[1] * 10 + dist_mod10[1];
        const int8_t d2 = dist_div10[2] * 10 + dist_mod10[2];

        if (d0 >= d1 && d0 >= d2) {
            places[0] = 3;
        }
        else
        if (d0 < d1 && d0 < d2) {
            places[0] = 0;
        }
        else {
            places[0] = 1;
        }

        if (d1 >= d0 && d1 >= d2) {
            places[1] = 3;
        }
        else
        if (d1 < d0 && d1 < d2) {
            places[1] = 0;
        }
        else {
            places[1] = 1;
        }

        if (d2 >= d0 && d2 >= d1) {
            places[2] = 3;
        }
        else
        if (d2 < d0 && d2 < d1) {
            places[2] = 0;
        }
        else {
            places[2] = 1;
        }
    }

    void randomize() {
        turns_left = 15;

        order = all_permutations[fast_rand() % 24];

        for (int i = 0; i < 3; i++) {
            dist_div10[i] = 0;
            dist_mod10[i] = 0;
            risk[i] = 0;
        }

        end = false;
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
                
                dist_mod10[i] += 2;

                if (index == 0) {
                    dist_mod10[i]--;
                    
                    if (risk[i] > 0)
                        risk[i]--;
                }
                else
                if (index == 2) {
                    risk[i]++;
                }
                else
                if (index == 3) {
                    dist_mod10[i]++;
                    risk[i] += 2;
                }

                if (dist_mod10[i] >= 10) {
                    dist_mod10[i] -= 10;
                    dist_div10[i]++;
                }
            }
        }

        if (risk[0] >= 0 && (dist_mod10[0] == dist_mod10[1] || dist_mod10[0] == dist_mod10[2])) {
            risk[0] += 2;
        }

        if (risk[1] >= 0 && (dist_mod10[1] == dist_mod10[0] || dist_mod10[1] == dist_mod10[2])) {
            risk[1] += 2;
        }

        if (risk[2] >= 0 && (dist_mod10[2] == dist_mod10[0] || dist_mod10[2] == dist_mod10[1])) {
            risk[2] += 2;
        }

        for (int i = 0; i < 3; i++) {
            if (risk[i] >= 5) {
                risk[i] = -2;
            }
        }

        if (turns_left == 1 /*|| turns_done == 10*/) {
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

    inline bool playable(const int8_t player_idx) const {
        return true; // return risk[player_idx] >= 0;
    }

    // maybe if for not being stun'ed
    // inline uint8_t greedy_moves(const int8_t player_idx) const {
    //     if (end || risk[player_idx] < 0)
    //         return 0;

    //     // std::cerr << "order: " << int(order) << ' ' << int(risk[player_idx]) << '\n';
    //     if (risk[player_idx] + 2 < 5) {
    //         // std::cerr << "take risk\n";
    //         return uint8_t(1) << last_in_permutation[order];
    //         // return uint8_t(1) << ((order >> (2 * 3)) & 3); // if have risk + 2 < 5 then rush 3
    //     }
    //     return uint8_t(1) << second_in_permutation[order];
    //     // return uint8_t(1) << ((order >> (1 * 2)) & 3); // else go 2
    // }
};

#endif // ROLLER_SPEED_SKATING