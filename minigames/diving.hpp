#ifndef DIVING_HPP
#define DIVING_HPP

#include "../const.hpp"

struct Diving {

    int goal[DIVING_LENGTH];
    int goal_index;

    int score[3];
    int combo[3];

    int places[3];

    bool end;

    void play(const int* move) {
        if (end) {
            return;
        }
        
        for (int i = 0; i < 3; i++) {
            if (move[i] == goal[goal_index]) {
                combo[i]++;
                score[i] += combo[i];
            }
            else {
                combo[i] = 0;
            }
        }

        if (goal_index == 0) {
            end = true;

            for (int i = 0; i < 3; i++) {
                if (score[i] >= score[(i + 1) % 3] && score[i] >= score[(i + 2) % 3]) {
                    places[i] = 1;
                }
                else
                if (score[i] < score[(i + 1) % 3] && score[i] < score[(i + 2) % 3]) {
                    places[i] = 2;
                }
                else {
                    places[i] = 3;
                }
            }
        }
        else {
            goal_index--;
        }
    }

    bool proven_win(int p_idx) const {
        return false;
    }

    bool proven_lost(int p_idx) const {
        return false;
    }
};

#endif // DIVING_HPP