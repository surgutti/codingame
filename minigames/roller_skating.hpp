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

    inline void generate_places(float* places) {
        if (end) {
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
        else {
            const int dist0 = dist_div10[0] * 10 + dist_mod10[0];
            const int dist1 = dist_div10[1] * 10 + dist_mod10[1];
            const int dist2 = dist_div10[2] * 10 + dist_mod10[2];

            float fst_p0 = fst_dp[turns_left][dist0][risk[0] + 2][dist1][risk[1] + 2] *
                           fst_dp[turns_left][dist0][risk[0] + 2][dist2][risk[2] + 2];

            float trd_p0 = snd_dp[turns_left][dist0][risk[0] + 2][dist1][risk[1] + 2] *
                           snd_dp[turns_left][dist0][risk[0] + 2][dist2][risk[2] + 2];
        
            float snd_p0 = 1.0 - fst_p0 - trd_p0;

            places[0] = 3 * fst_p0 + 1 * snd_p0;


            float fst_p1 = fst_dp[turns_left][dist1][risk[1] + 2][dist0][risk[0] + 2] *
                           fst_dp[turns_left][dist1][risk[1] + 2][dist2][risk[2] + 2];

            float trd_p1 = snd_dp[turns_left][dist1][risk[1] + 2][dist0][risk[0] + 2] *
                           snd_dp[turns_left][dist1][risk[1] + 2][dist2][risk[2] + 2];;
        
            float snd_p1 = 1.0 - fst_p1 - trd_p1;

            places[1] = 3 * fst_p1 + 1 * snd_p1;

            float fst_p2 = fst_dp[turns_left][dist2][risk[2] + 2][dist0][risk[0] + 2] *
                           fst_dp[turns_left][dist2][risk[2] + 2][dist1][risk[1] + 2];

            float trd_p2 = snd_dp[turns_left][dist2][risk[2] + 2][dist0][risk[0] + 2] *
                           snd_dp[turns_left][dist2][risk[2] + 2][dist1][risk[1] + 2];
        
            float snd_p2 = 1.0 - fst_p2 - trd_p2;

            places[2] = 3 * fst_p2 + 1 * snd_p2;
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

    static long double fst_dp[16][32][7][32][7];
    static long double snd_dp[16][32][7][32][7];

    static void build_dp() {
        for (int a = 0; a < 32; a++) {
            for (int b = 0; b < 7; b++) {
                for (int c = 0; c < 32; c++) {
                    for (int d = 0; d < 7; d++) {
                        if (a >= c) {
                            fst_dp[0][a][b][c][d] = 1.0f;
                        }
                        else {
                            fst_dp[0][a][b][c][d] = 0.0f;    
                        }
                        
                        if (a < c) {
                            snd_dp[0][a][b][c][d] = 1.0f;
                        }
                        else {
                            snd_dp[0][a][b][c][d] = 0.0f;    
                        }
                    }
                }
            }
        }

        auto make_move = [&](int a, int b, int &c, int &d, int m) {
            if (b - 2 < 0) {
                c = a;
                d = b + 1;
            }
            else {
                if (m == 0) {
                    c = a + 1;
                    d = std::max(2, b - 1);
                }
                else
                if (m == 1) {
                    c = a + 2;
                    d = b;
                }
                else
                if (m == 2) {
                    c = a + 2;
                    d = b + 1;
                }
                else
                if (m == 3) {
                    c = a + 3;
                    d = b + 2;
                }
                else {
                    assert(false);
                }
            }
        };

        for (int k = 1; k <= 15; k++) {
            for (int a = 0; a <= 28; a++) {
                for (int b = 0; b < 7; b++) {
                    for (int c = 0; c <= 28; c++) {
                        for (int d = 0; d < 7; d++) {
                            fst_dp[k][a][b][c][d] = 0;
                            snd_dp[k][a][b][c][d] = 0;

                            for (int m0 = 0; m0 < 4; m0++) {
                                
                                for (int m1 = 0; m1 < 4; m1++) {

                                    int aa, bb;
                                    make_move(a, b, aa, bb, m0);

                                    int cc, dd;
                                    make_move(c, d, cc, dd, m1);

                                    if (aa % 10 == cc % 10) {
                                        if (bb - 2 >= 0)
                                            bb += 2;
                                        
                                        if (dd - 2 >= 0)
                                            dd += 2;
                                    }

                                    if (bb - 2 >= 5) {
                                        bb = 0;
                                    }

                                    if (dd - 2 >= 5) {
                                        dd = 0;
                                    }

                                    fst_dp[k][a][b][c][d] += 0.0625 * fst_dp[k - 1][aa][bb][cc][dd];
                                    snd_dp[k][a][b][c][d] += 0.0625 * snd_dp[k - 1][aa][bb][cc][dd];
                                }
                            }
                        }
                    }
                }
            }
        }
    }

};

long double RollerSkating::fst_dp[16][32][7][32][7];
long double RollerSkating::snd_dp[16][32][7][32][7];

#endif // ROLLER_SPEED_SKATING