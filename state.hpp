#ifndef STATE_HPP
#define STATE_HPP

#include "minigames/hurdle_race.hpp"
#include "minigames/archery.hpp"
#include "minigames/roller_skating.hpp"
#include "minigames/diving.hpp"

#include "utils.hpp"

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

struct State {

    HurdleRace hurdle_race;
    uint8_t hurdle_race_score[3];

    Archery archery;
    uint8_t archery_score[3];

    RollerSkating roller_skating;
    uint8_t roller_skating_score[3];

    Diving diving;
    uint8_t diving_score[3];

    int turn;

    bool is_terminal() const {
        return (hurdle_race.end &&
                archery.end &&
                roller_skating.end &&
                diving.end) ||
               turn >= 100;
    }

    // return how much does a player earn from games
    void get_stats(float& r0, float& r1, float& r2) {

        int8_t places[3];

        if (hurdle_race.end) {
            hurdle_race.generate_places(places);
            for (int i = 0; i < 3; i++) {
                hurdle_race_score[i] += places[i];
            }
        }

        if (archery.end) {
            archery.generate_places(places);
            for (int i = 0; i < 3; i++) {
                archery_score[i] += places[i];
            }
        }

        if (roller_skating.end) {
            roller_skating.generate_places(places);
            for (int i = 0; i < 3; i++) {
                roller_skating_score[i] += places[i];
            }
        }

        if (diving.end) {
            diving.generate_places(places);
            for (int i = 0; i < 3; i++) {
                diving_score[i] += places[i];
            }
        }

        int score0 = std::max<int>(1, hurdle_race_score[0]) * std::max<int>(1, archery_score[0]) * std::max<int>(1, roller_skating_score[0]) * std::max<int>(1, diving_score[0]);
        int score1 = std::max<int>(1, hurdle_race_score[1]) * std::max<int>(1, archery_score[1]) * std::max<int>(1, roller_skating_score[1]) * std::max<int>(1, diving_score[1]);
        int score2 = std::max<int>(1, hurdle_race_score[2]) * std::max<int>(1, archery_score[2]) * std::max<int>(1, roller_skating_score[2]) * std::max<int>(1, diving_score[2]);

        // maybe change the enemy to win with him?
        // get some values from gameplay? (with whom likely to win at the end)
        int sum = score0 + score1 + score2;

        // maybe the place could be weighted after few turns (on the beginning just focus on own scores?)

        r0 = (float) (score0 - score1 - score2) / sum; // + (score0 > score1 && score0 > score2) - (score0 < score1 && score0 < score2);
        r1 = (float) (score1 - score0 - score2) / sum; // + (score1 > score0 && score1 > score2) - (score1 < score0 && score1 < score2);
        r2 = (float) (score2 - score0 - score1) / sum; // + (score2 > score0 && score2 > score1) - (score2 < score0 && score2 < score1);

        // float score0 = std::max<float>(1, hurdle_race_score[0]) * std::max<float>(0.95, archery_score[0]) * std::max<float>(0.93, roller_skating_score[0]) * std::max<float>(1, diving_score[0]);
        // float score1 = std::max<float>(1, hurdle_race_score[1]) * std::max<float>(0.95, archery_score[1]) * std::max<float>(0.93, roller_skating_score[1]) * std::max<float>(1, diving_score[1]);
        // float score2 = std::max<float>(1, hurdle_race_score[2]) * std::max<float>(0.95, archery_score[2]) * std::max<float>(0.93, roller_skating_score[2]) * std::max<float>(1, diving_score[2]);

        // float sum = score0 + score1 + score2;

        // r0 = (float) (score0 - score1 - score2) / sum;
        // r1 = (float) (score1 - score0 - score2) / sum;
        // r2 = (float) (score2 - score0 - score1) / sum;
    }

    void init(const std::vector<std::string>& gpu,
              const std::vector<std::vector<int>> reg) {

        if (gpu[0] == "GAME_OVER") {
            hurdle_race.end = true;
        }
        else { // hurdle_race
            hurdle_race.track = 0;
            for (int i = 0; i < (int) gpu[0].size(); i++) {
                if (gpu[0][i] == '#') {
                    hurdle_race.track |= uint32_t(1) << i;
                }
            }

            for (int i = 0; i < 3; i++) {
                hurdle_race.pos[i] = reg[0][i];
                hurdle_race.stun[i] = reg[0][i + 3];
            }

            hurdle_race.end = false;

            hurdle_race.build_dp();
        }

        if (gpu[1] == "GAME_OVER") {
            archery.end = true;
        }
        else { // archery
            archery.wind_index = (int) gpu[1].size() - 1;
            
            for (int i = 0; i < (int) gpu[1].size(); i++) {
                archery.wind[archery.wind_index - i] = int(gpu[1][i] - '0');
            }

            for (int i = 0; i < 3; i++) {
                archery.x[i] = reg[1][2 * i + 0];
                archery.y[i] = reg[1][2 * i + 1];
            }

            archery.end = false;

            archery.build_dp();
        }

        if (gpu[2] == "GAME_OVER") {
            roller_skating.end = true;
        }
        else { // roller_skating
            roller_skating.order = 0;
            for (int i = 0; i < 4; i++) {
                roller_skating.order |= uint8_t(i) << (to_move_index(gpu[2][i]) << 1);
            }

            for (int i = 0; i < 3; i++) {
                roller_skating.dist[i] = reg[2][i];
                roller_skating.risk[i] = reg[2][i + 3];
            }

            roller_skating.turns_left = reg[2][6];
            // roller_skating.turns_done = 0;

            roller_skating.end = false;
        }

        if (gpu[3] == "GAME_OVER") {
            diving.end = true;
        }
        else { // diving
            diving.goals_left = (int8_t) gpu[3].size();

            diving.goal = 0;
            for (int i = 0; i < (int) gpu[3].size(); i++) {
                diving.goal |= uint32_t(to_move_index(gpu[3][i])) << (2 * i);
            }
            
            for (int i = 0; i < 3; i++) {
                diving.score[i] = reg[3][i];
                diving.combo[i] = reg[3][i + 3];
            }

            diving.end = false;
        }
    }
    
    inline bool still_playing() const {
        return !hurdle_race.end ||
               !archery.end ||
               !roller_skating.end ||
               !diving.end;
    }

    inline void greedy_moves(int8_t* move) const {
        for (int i = 0; i < 3; i++) {

            // std::cerr << "player: " << i << '\n';

            int eval[4] = {0, 0, 0, 0};

            if (hurdle_race.playable(i)) {
                uint8_t good_moves = hurdle_race.greedy_moves(i);
                for (int j = 0; j < 4; j++) {
                    if (good_moves & (1 << j)) {
                        eval[j] += int(archery_score[i]) * roller_skating_score[i] * diving_score[i] + 1;
                    }
                }
            }
            // else {
            //     std::cerr << "hurdles useless\n";
            // }

            if (archery.playable(i)) {
                uint8_t good_moves = archery.greedy_moves(i);
                for (int j = 0; j < 4; j++) {
                    if (good_moves & (1 << j)) {
                        eval[j] += int(hurdle_race_score[i]) * roller_skating_score[i] * diving_score[i] + 1;
                    }
                }
            }
            // else {
            //     std::cerr << "archery useless\n";
            // }

            if (roller_skating.playable(i)) {
                uint8_t good_moves = roller_skating.greedy_moves(i);
                for (int j = 0; j < 4; j++) {
                    if (good_moves & (1 << j)) {
                        eval[j] += int(hurdle_race_score[i]) * archery_score[i] * diving_score[i] + 1;
                    }
                }
            }
            // else {
            //     std::cerr << "skating useless\n";
            // }

            if (diving.playable(i)) {
                uint8_t good_moves = diving.greedy_moves(i);
                for (int j = 0; j < 4; j++) {
                    if (good_moves & (1 << j)) {
                        eval[j] += int(hurdle_race_score[i]) * archery_score[i] * roller_skating_score[i] + 1;
                    }
                }
            }
            // else {
            //     std::cerr << "diving useless\n";
            // }

            // std::cerr << "eval: ";
            // for (int j = 0; j < 4; j++) {
                // std::cerr << int(eval[j]) << ' ';
            // }
            // std::cerr << '\n';

            int best_score = -1;
            for (int j = 0; j < 4; j++) {
                if (best_score < eval[j]) {
                    best_score = eval[j];
                    move[i] = j;
                }
                else
                if (best_score == eval[j] && fast_rand() & 1) {
                    best_score = eval[j];
                    move[i] = j;
                }
            }
        }
    }

    void play_greedy() {
        int8_t move[3];

        greedy_moves(move);
        play(move[0], move[1], move[2]);
    }

    void play(int8_t p0, int8_t p1, int8_t p2) {
        const int8_t move[3] = {p0, p1, p2};

        hurdle_race.play(move);
        archery.play(move);
        roller_skating.play(move);
        diving.play(move);
        turn++;
    }

    void debug() const {
        std::cerr << "state: " << hurdle_race.end << ' ' << archery.end << ' ' << roller_skating.end << ' ' << diving.end << '\n';
        hurdle_race.debug();
        archery.debug();
        roller_skating.debug();
        diving.debug();
    }

};

#endif // STATE_HPP