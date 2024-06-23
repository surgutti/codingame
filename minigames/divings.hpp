#ifndef DIVINGS_HPP
#define DIVINGS_HPP

#include "../const.hpp"
#include "../random.hpp"

#include <algorithm>
#include <vector>
#include <utility>

struct Divings {

    int32_t goal;
    int8_t goals_left;

    uint8_t score[3];
    int8_t combo[3];

    bool end;

    inline bool is_useless(int8_t player_idx) const {
        return false;
    }

    inline uint8_t greedy_moves(int8_t player_idx) const {
        return (goal & 3);
    }

    bool operator== (const Divings &other) const {
        if (end != other.end)
            return false;
        
        if (end)
            return true;

        for (int i = 0; i < 3; i++) {
            if (score[i] != other.score[i])
                return false;
            
            if (combo[i] != other.combo[i])
                return false;
        }

        return goal == other.goal;
    }

    void debug() const {
        std::cerr << "goals_left: " << int(goals_left) << '\n';
        std::cerr << "goal: ";
        for (int i = 0; i < goals_left; i++) {
            std::cerr << int((goal >> (i * 2)) & 3) << ' ';
        }
        std::cerr << '\n';
        for (int i = 0; i < 3; i++) {
            std::cerr << "i: " << i << " => " << int(score[i]) << ' ' << int(combo[i]) << '\n';
        }
    }

    inline int expected_end() const {
        return goals_left;
    }

    inline void generate_places(float* places) const {
        if (score[0] >= score[1] && score[0] >= score[2]) {
            places[0] = 3;
        }
        else
        if (score[0] < score[1] && score[0] < score[2]) {
            places[0] = 0;
        }
        else {
            places[0] = 1;
        }

        if (score[1] >= score[0] && score[1] >= score[2]) {
            places[1] = 3;
        }
        else
        if (score[1] < score[0] && score[1] < score[2]) {
            places[1] = 0;
        }
        else {
            places[1] = 1;
        }

        if (score[2] >= score[0] && score[2] >= score[1]) {
            places[2] = 3;
        }
        else
        if (score[2] < score[0] && score[2] < score[1]) {
            places[2] = 0;
        }
        else {
            places[2] = 1;
        }
    }

    void randomize() {
        goals_left = 12 + fast_rand() % 4;
        goal = fast_rand();

        for (int i = 0; i < 3; i++) {
            score[i] = 0;
            combo[i] = 0;
        }

        end = false;
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

    inline bool playable(int8_t player_idx) const {
        const int8_t enemy1_idx = (player_idx + 1) % 3;
        const int8_t enemy2_idx = (player_idx + 2) % 3;

        int best_player = score[player_idx] + combo[player_idx] * goals_left + ((int(goals_left) * (goals_left + 1)) >> 1);
        int worst_player = score[player_idx];

        int best_enemy1 = score[enemy1_idx] + combo[enemy1_idx] * goals_left + ((int(goals_left) * (goals_left + 1)) >> 1);
        int worst_enemy1 = score[enemy1_idx];

        int best_enemy2 = score[enemy2_idx] + combo[enemy2_idx] * goals_left + ((int(goals_left) * (goals_left + 1)) >> 1);
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

        return true;
    }

};

#endif // DIVINGS_HPP