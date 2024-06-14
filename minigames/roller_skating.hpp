#ifndef ROLLER_SKATING
#define ROLLER_SKATING

#include "../random.hpp"

#include <iostream>
#include <algorithm>

struct RollerSkating {

    int8_t turns_left;

    int8_t dist[3];
    int8_t risk[3];

    int8_t places[3];

    int8_t order[4];

    bool end;

    void debug() const {
        std::cerr << "TURNS LEFT: " << int(turns_left) << '\n';
        for (int i = 0; i < 3; i++) {
            std::cerr << "i: " << i << ' ' << int(dist[i]) << ' ' << int(risk[i]) << '\n';
        }
        std::cerr << "order: ";
        for (int i = 0; i < 4; i++) {
            std::cerr << int(order[i]) << ' ';
        }
        std::cerr << '\n';
    }

    void play(const int8_t* move) {
        if (end) {
            return;
        }

        for (int i = 0; i < 3; i++) {
            if (risk[i] < 0) {
                risk[i]++;
            }
            else
            if (order[move[i]] == 0) {
                dist[i] += 1;
                risk[i] -= 1;
            }
            else
            if (order[move[i]] == 1) {
                dist[i] += 2;
            }
            else
            if (order[move[i]] == 2) {
                dist[i] += 2;
                risk[i] += 1;
            }
            else {
                dist[i] += 3;
                risk[i] += 2;
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

        if (turns_left == 0) {
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

            end = true;
        }
        else {
            turns_left--;

            // new order
            std::swap(order[3], order[fast_rand() % 4]);
            std::swap(order[2], order[fast_rand() % 3]);
            std::swap(order[1], order[fast_rand() % 2]);
        }
    }

};

#endif // ROLLER_SPEED_SKATING