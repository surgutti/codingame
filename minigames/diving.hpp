#ifndef DIVING_HPP
#define DIVING_HPP

#include "../const.hpp"

struct Diving {

    int32_t goal;
    int8_t goals_left;

    uint8_t score[3];
    int8_t combo[3];

    bool end;

    void debug() const {
        std::cerr << "goals_left: " << goals_left << '\n';
        std::cerr << "goal: ";
        for (int i = 0; i < goals_left; i++) {
            std::cerr << int((goal >> (i * 2)) & 3) << ' ';
        }
        std::cerr << '\n';
        for (int i = 0; i < 3; i++) {
            std::cerr << "i: " << i << " => " << int(score[i]) << ' ' << int(combo[i]) << '\n';
        }
    }

    void generate_places(int8_t* places) const {
        for (int i = 0; i < 3; i++) {
            if (score[i] >= score[(i + 1) % 3] && score[i] >= score[(i + 2) % 3]) {
                places[i] = 3;
            }
            else
            if (score[i] < score[(i + 1) % 3] && score[i] < score[(i + 2) % 3]) {
                places[i] = 0;
            }
            else {
                places[i] = 1;
            }
        }
    }

    inline void play(const int8_t* move) {
        if (end) {
            return;
        }
        
        for (int i = 0; i < 3; i++) {
            if (move[i] == (goal & 3)) {
                combo[i]++;
                score[i] += combo[i];
            }
            else {
                combo[i] = 0;
            }
        }

        if (goals_left == 1) {
            end = true;
        }
        else {
            goal >>= 2;
            goals_left--;
        }
    }
};

#endif // DIVING_HPP