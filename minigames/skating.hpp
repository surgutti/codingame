#ifndef SKATING
#define SKATING

#include "../random.hpp"

#include <iostream>
#include <algorithm>

struct Skating {

    int8_t risk[3];
    int8_t dist_div10[3];
    int8_t dist_mod10[3];
    
    int8_t turns_left;
    uint8_t order;

    bool end;

    bool is_useless(int8_t player_idx) const {
        return risk[player_idx] < 0;
    }

    uint8_t greedy_moves(int8_t player_idx) const {
        return 0b1011;
    }

    bool operator== (const Skating &other) const {
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

};
#endif // SKATING