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

    bool in_waiting(const int8_t player_idx) const {
        return false;
    }

    inline bool playable(int8_t player_idx) const {
        const int8_t enemy1_idx = (player_idx + 1) % 3;
        const int8_t enemy2_idx = (player_idx + 2) % 3;

        int best_player = score[player_idx] + combo[player_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1);
        int worst_player = score[player_idx];

        int best_enemy1 = score[enemy1_idx] + combo[enemy1_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1);
        int worst_enemy1 = score[enemy1_idx];

        int best_enemy2 = score[enemy2_idx] + combo[enemy2_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1);
        int worst_enemy2 = score[enemy2_idx];

        if (worst_player >= best_enemy1 && worst_player >= best_enemy2) {
            return false; // inevitable 1st place
        }

        if (best_player < worst_enemy1 && best_player < worst_enemy2) {
            return false; // inevitable 3rd place
        }

        if (worst_player >= best_enemy1 && best_player < worst_enemy2) {
            return false; // inevitable 2nd place
        }

        if (best_player < worst_enemy1 && worst_player >= best_enemy2) {
            return false; // inevitable 2nd place
        }

        // assert (int(goals_left) * (goals_left + 1) < 256);
        // if (score[player_idx] >= score[enemy1_idx] + combo[enemy1_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1) &&
        //     score[player_idx] >= score[enemy2_idx] + combo[enemy2_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1)) {
        //     return false; // inevitable 1st place
        // }

        // assert (int(goals_left) * (goals_left + 1) < 256);
        // if (score[player_idx] + combo[player_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1) < score[enemy1_idx] &&
        //     score[player_idx] + combo[player_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1) < score[enemy2_idx]) {
        //     return false; // inevitable 3rd place
        // }

        return true;
    }

    inline uint8_t greedy_moves(const int8_t player_idx) const {
        if (end)
            return 0;
        
        return uint8_t(1) << (goal & 3);
    }
};

#endif // DIVING_HPP