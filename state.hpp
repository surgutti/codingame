#ifndef STATE_HPP
#define STATE_HPP

#include "minigames/hurdle_race.hpp"
#include "minigames/archery.hpp"
#include "minigames/roller_skating.hpp"
#include "minigames/diving.hpp"

#include "utils.hpp"

#include <algorithm>
#include <vector>
#include <string>

struct State {

    HurdleRace hurdle_race;
    int hurdle_race_score[3];

    Archery archery;
    int archery_score[3];

    // RollerSkating roller_skating;
    int roller_skating_score[3];

    Diving diving;
    int diving_score[3];

    bool is_terminal() const {
        return hurdle_race.end &&
               archery.end &&
               diving.end;
    }

    int place_to_score(int place) const {
        if (place == 1)
            return 3;
        if (place == 2)
            return 1;
        return 0;
    }

    // return how much does a player earn from games
    void get_stats(float& r0, float& r1, float& r2) {

        for (int i = 0; i < 3; i++) {
            hurdle_race_score[i] += place_to_score(hurdle_race.places[i]);
            archery_score[i] += place_to_score(archery.places[i]);
            diving_score[i] += place_to_score(diving.places[i]);
        }

        r0 = hurdle_race_score[0] * archery_score[0] * diving_score[0];
        r0 = hurdle_race_score[0] * archery_score[0] * diving_score[0];
        r0 = hurdle_race_score[0] * archery_score[0] * diving_score[0]; 

        r0 = place_to_score(hurdle_race.places[0]) * (archery_score[0] + roller_skating_score[0] + diving_score[0]) +
             place_to_score(archery.places[0]) * (hurdle_race_score[0] + roller_skating_score[0] + diving_score[0]) +
             place_to_score(diving.places[0]) * (hurdle_race_score[0] + archery_score[0] + roller_skating_score[0]);

        r1 = place_to_score(hurdle_race.places[1]) * (archery_score[1] + roller_skating_score[1] + diving_score[1]) +
             place_to_score(archery.places[1]) * (hurdle_race_score[1] + roller_skating_score[1] + diving_score[1]) +
             place_to_score(diving.places[1]) * (hurdle_race_score[1] + archery_score[1] + roller_skating_score[1]);

        r2 = place_to_score(hurdle_race.places[2]) * (archery_score[2] + roller_skating_score[2] + diving_score[2]) +
             place_to_score(archery.places[2]) * (hurdle_race_score[2] + roller_skating_score[2] + diving_score[2]) +
             place_to_score(diving.places[2]) * (hurdle_race_score[2] + archery_score[2] + roller_skating_score[2]);
        
        int sum = r0 + r1 + r2;

        if (sum != 0) {
            r0 /= sum;
            r1 /= sum;
            r2 /= sum;
        }
    }

    void init(const std::vector<std::string>& gpu,
              const std::vector<std::vector<int>> reg) {

        if (gpu[0] == "GAME_OVER") {
            hurdle_race.end = true;
        }
        else { // hurdle_race
            for (int i = 0; i < (int) gpu[0].size(); i++) {
                hurdle_race.track[i] = gpu[0][i];
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

                archery.scores[i] = 0;
                archery.places[i] = -1;
            }

            archery.end = false;
        }

        if (gpu[2] == "GAME_OVER") {

        }
        else { // roller_skating

        }

        if (gpu[3] == "GAME_OVER") {
            diving.end = true;
        }
        else { // diving
            diving.goal_index = (int) gpu[3].size() - 1;
            for (int i = 0; i < (int) gpu[3].size(); i++) {
                diving.goal[diving.goal_index - i] = to_move_index(gpu[3][i]);
            }
        }
    }

    void play(int p0, int p1, int p2) {
        const int move[3] = {p0, p1, p2};

        hurdle_race.play(move);
        archery.play(move);
        // roller_skating.play(move);
        diving.play(move);
    }

};

#endif // STATE_HPP