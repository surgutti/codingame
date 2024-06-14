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

    bool is_terminal() const {
        return hurdle_race.end &&
               archery.end &&
               roller_skating.end &&
               diving.end;
    }

    // return how much does a player earn from games
    void get_stats(float& r0, float& r1, float& r2) {

        for (int i = 0; i < 3; i++) {
            hurdle_race_score[i] += hurdle_race.places[i];
            archery_score[i] += archery.places[i];
            roller_skating_score[i] += roller_skating.places[i];
            diving_score[i] += diving.places[i];
        }

        int score0 = std::max<int>(1, hurdle_race_score[0]) * std::max<int>(1, archery_score[0]) * std::max<int>(1, roller_skating_score[0]) * std::max<int>(1, diving_score[0]);
        int score1 = std::max<int>(1, hurdle_race_score[1]) * std::max<int>(1, archery_score[1]) * std::max<int>(1, roller_skating_score[1]) * std::max<int>(1, diving_score[1]);
        int score2 = std::max<int>(1, hurdle_race_score[2]) * std::max<int>(1, archery_score[2]) * std::max<int>(1, roller_skating_score[2]) * std::max<int>(1, diving_score[2]);

        int sum = score0 + score1 + score2;

        r0 = (float) (score0 - score1 - score2) / sum;
        r1 = (float) (score1 - score0 - score2) / sum;
        r2 = (float) (score2 - score0 - score1) / sum;
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
                hurdle_race.places[i] = -1;
            }

            hurdle_race.end = false;
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
                archery.places[i] = -1;
            }

            archery.end = false;
        }

        if (gpu[2] == "GAME_OVER") {
            roller_skating.end = true;
        }
        else { // roller_skating
            for (int i = 0; i < 24; i++) {
                bool ok = true;
                for (int j = 0; j < 4; j++) {
                    if (all_permutations[i][j] != to_move_index(gpu[2][j])) {
                        ok = false;
                        break;
                    }
                }

                if (ok) {
                    roller_skating.order = all_permutations[i];
                    break;
                }
            }

            for (int i = 0; i < 3; i++) {
                roller_skating.dist[i] = reg[2][i];
                roller_skating.risk[i] = reg[2][i + 3];
            }

            roller_skating.turns_left = reg[2][6];

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
                diving.places[i] = -1;
            }

            diving.end = false;
        }
    }

    void play(int8_t p0, int8_t p1, int8_t p2) {
        const int8_t move[3] = {p0, p1, p2};

        hurdle_race.play(move);
        archery.play(move);
        roller_skating.play(move);
        diving.play(move);
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