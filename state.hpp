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
    Archery archery;
    RollerSkating roller_skating;
    Diving diving;

    float hurdle_race_score[3];
    int8_t hurdle_race_left;
    
    float archery_score[3];
    int8_t archery_left;

    float roller_skating_score[3];
    int8_t roller_skating_left;

    float diving_score[3];
    int8_t diving_left;

    int8_t turn;

    inline void apply_places() {

        float places[3];

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
    }

    bool operator== (const State &other) const {
        if (!hurdle_race.end && !other.hurdle_race.end &&
            !(hurdle_race == other.hurdle_race)) {
            // std::cerr << "HURDLE FAIL\n";
            return false;
        }

        for (int i = 0; i < 3; i++) {
            if (hurdle_race_score[i] != other.hurdle_race_score[i]) {
                // std::cerr << "HURDLE SCORE FAIL\n";
                return false;
            }
        }

        if (!archery.end && !other.archery.end &&
            !(archery == other.archery)) {
            // std::cerr << "ARCHERY FAIL\n";
            return false;
        }

        for (int i = 0; i < 3; i++) {
            if (archery_score[i] != other.archery_score[i]) {
                // std::cerr << "ARCHERY SCORE FAIL\n";
                return false;
            }
        }

        if (!roller_skating.end && !other.roller_skating.end &&
            !(roller_skating == other.roller_skating)) {
            // std::cerr << "SKATING FAIL\n";
            return false;
        }
        
        for (int i = 0; i < 3; i++) {
            if (roller_skating_score[i] != other.roller_skating_score[i]) {
                // std::cerr << "SKATING SCORE FAIL\n";
                return false;
            }
        }

        if (!diving.end && !other.diving.end &&
            !(diving == other.diving)) {
            // std::cerr << "DIVING FAIL\n";
            return false;
        }

        for (int i = 0; i < 3; i++) {
            if (diving_score[i] != other.diving_score[i]) {
                // std::cerr << "DIVING SCORE FAIL\n";
                return false;
            }
        }

        if (turn != other.turn) {
            // std::cerr << "TURN FAIL\n";
            return false;
        }

        return true;
    }

    inline bool is_terminal() const {
        return (hurdle_race.end && hurdle_race_left == 0 &&
                archery.end && archery_left == 0 &&
                roller_skating.end && roller_skating_left == 0 &&
                diving.end && diving_left == 0) ||
                turn >= 100;
    }

    inline bool is_rollout_terminal() const {
        return (hurdle_race.end &&
                archery.end &&
                roller_skating.end &&
                diving.end) ||
                turn >= 100;
    }

    // return how much does a player earn from games
    inline void get_stats(float* rewards) {

        apply_places();

        int scores[3] = {1, 1, 1};

        for (int i = 0; i < 3; i++) {
            if (hurdle_race_score[i] > 0)
                scores[i] *= hurdle_race_score[i];

            if (archery_score[i] > 0)
                scores[i] *= archery_score[i];
            
            if (roller_skating_score[i] > 0)
                scores[i] *= roller_skating_score[i];
            
            if (diving_score[i] > 0)
                scores[i] *= diving_score[i];
        }

        const int sum = (scores[0] + scores[1] + scores[2]);

        // {
        //     int max_enemy = std::max<int>(scores[1], scores[2]);
        //     int min_enemy = std::min<int>(scores[1], scores[2]);

        //     rewards[0] = float(scores[0] - COEFFICIENT1 * max_enemy - COEFFICIENT2 * min_enemy) / (scores[0] + COEFFICIENT1 * max_enemy + COEFFICIENT2 * min_enemy);
        // }

        // {
        //     int max_enemy = std::max<int>(scores[0], scores[2]);
        //     int min_enemy = std::min<int>(scores[0], scores[2]);

        //     rewards[1] = float(scores[1] - COEFFICIENT1 * max_enemy - COEFFICIENT2 * min_enemy) / (scores[1] + COEFFICIENT1 * max_enemy + COEFFICIENT2 * min_enemy);
        // }

        // {
        //     int max_enemy = std::max<int>(scores[0], scores[1]);
        //     int min_enemy = std::min<int>(scores[0], scores[1]);

        //     rewards[2] = float(scores[2] - COEFFICIENT1 * max_enemy - COEFFICIENT2 * min_enemy) / (scores[2] + COEFFICIENT1 * max_enemy + COEFFICIENT2 * min_enemy);
        // }

        rewards[0] = float(scores[0] - scores[1] - scores[2]) / sum;
        rewards[1] = float(scores[1] - scores[0] - scores[2]) / sum;
        rewards[2] = float(scores[2] - scores[0] - scores[1]) / sum;

        // r0 = (float) (score0 - score1 - score2) / (score0 + score1 + score2); // + (score0 > score1 && score0 > score2) - (score0 < score1 && score0 < score2);
        // r1 = (float) (score1 - score0 - score2) / (score1 + score0 + score2); // + (score1 > score0 && score1 > score2) - (score1 < score0 && score1 < score2);
        // r2 = (float) (score2 - score0 - score1) / (score2 + score0 + score1); // + (score2 > score0 && score2 > score1) - (score2 < score0 && score2 < score1);
        
        // r0 = (float) (0.4 * score0 - score1 - score2) / (0.4 * score0 + score1 + score2); // + (score0 > score1 && score0 > score2) - (score0 < score1 && score0 < score2);
        // r1 = (float) (0.4 * score1 - score0 - score2) / (0.4 * score1 + score0 + score2); // + (score1 > score0 && score1 > score2) - (score1 < score0 && score1 < score2);
        // r2 = (float) (0.4 * score2 - score0 - score1) / (0.4 * score2 + score0 + score1); // + (score2 > score0 && score2 > score1) - (score2 < score0 && score2 < score1);
        
        // r0 += 0.01 * ((score0 > score1 && score0 > score2) - (score0 < score1 && score0 < score2));
        // r1 += 0.01 * ((score1 > score0 && score1 > score2) - (score1 < score0 && score1 < score2));
        // r2 += 0.01 * ((score2 > score0 && score2 > score1) - (score2 < score0 && score2 < score1));

        // r0 += 0.01 * (hurdle_race_score[0] - hurdle_race_score[1] - hurdle_race_score[2]);
        // r1 += 0.01 * (hurdle_race_score[1] - hurdle_race_score[0] - hurdle_race_score[2]);
        // r2 += 0.01 * (hurdle_race_score[2] - hurdle_race_score[0] - hurdle_race_score[1]);

        // r0 += 0.01 * (archery_score[0] - archery_score[1] - archery_score[2]);
        // r1 += 0.01 * (archery_score[1] - archery_score[0] - archery_score[2]);
        // r2 += 0.01 * (archery_score[2] - archery_score[0] - archery_score[1]);

        // r0 += 0.01 * (roller_skating_score[0] - roller_skating_score[1] - roller_skating_score[2]);
        // r1 += 0.01 * (roller_skating_score[1] - roller_skating_score[0] - roller_skating_score[2]);
        // r2 += 0.01 * (roller_skating_score[2] - roller_skating_score[0] - roller_skating_score[1]);

        // r0 += 0.01 * (diving_score[0] - diving_score[1] - diving_score[2]);
        // r1 += 0.01 * (diving_score[1] - diving_score[0] - diving_score[2]);
        // r2 += 0.01 * (diving_score[2] - diving_score[0] - diving_score[1]);

        // maybe change the enemy to win with him?
        // get some values from gameplay? (with whom likely to win at the end)
        // int sum = score0 + score1 + score2;

        // maybe the place could be weighted after few turns (on the beginning just focus on own scores?)

        // if (turn == 100) {
        //     // if simulated to the end of the game, just get the current leaderboard

        // }
        // else {
            // assuming that opponents are litle against me

        // const float score0_log = fastlogf(score0);
        // const float score1_log = fastlogf(score1);
        // const float score2_log = fastlogf(score2);

        // const float turn_log = fastlogf(turn + 1);

        // r0 = (float) COEFFICIENT1 * (score0_log - score1_log - score2_log) / (score0_log + score1_log + score2_log);
        // r1 = (float) COEFFICIENT1 * (score1_log - score0_log - score2_log) / (score1_log + score0_log + score2_log);
        // r2 = (float) COEFFICIENT1 * (score2_log - score0_log - score1_log) / (score2_log + score0_log + score1_log);

        // r0 = (float) (score0 - score1 - score2) / (score0 + score1 + score2);
        // r1 = (float) (score1 - score0 - score2) / (score1 + score0 + score2);
        // r2 = (float) (score2 - score0 - score1) / (score2 + score0 + score1);
        
        // r0 += COEFFICIENT3 * turn_log * ((score0 > score1 && score0 > score2) - (score0 < score1 && score0 < score2));
        // r1 += COEFFICIENT3 * turn_log * ((score1 > score0 && score1 > score2) - (score1 < score0 && score1 < score2));
        // r2 += COEFFICIENT3 * turn_log * ((score2 > score0 && score2 > score1) - (score2 < score0 && score2 < score1));

        // r0 += COEFFICIENT4 * (float) (hurdle_race_score[0] - hurdle_race_score[1] - hurdle_race_score[2]) / (1 + hurdle_race_score[0] + hurdle_race_score[1] + hurdle_race_score[2]);
        // r1 += COEFFICIENT4 * (float) (hurdle_race_score[1] - hurdle_race_score[0] - hurdle_race_score[2]) / (1 + hurdle_race_score[0] + hurdle_race_score[1] + hurdle_race_score[2]);
        // r2 += COEFFICIENT4 * (float) (hurdle_race_score[2] - hurdle_race_score[0] - hurdle_race_score[1]) / (1 + hurdle_race_score[0] + hurdle_race_score[1] + hurdle_race_score[2]);

        // r0 += COEFFICIENT4 * (float) (archery_score[0] - archery_score[1] - archery_score[2]) / (1 + archery_score[0] + archery_score[1] + archery_score[2]);
        // r1 += COEFFICIENT4 * (float) (archery_score[1] - archery_score[0] - archery_score[2]) / (1 + archery_score[0] + archery_score[1] + archery_score[2]);
        // r2 += COEFFICIENT4 * (float) (archery_score[2] - archery_score[0] - archery_score[1]) / (1 + archery_score[0] + archery_score[1] + archery_score[2]);

        // r0 += COEFFICIENT4 * (float) (roller_skating_score[0] - roller_skating_score[1] - roller_skating_score[2]) / (1 + roller_skating_score[0] + roller_skating_score[1] + roller_skating_score[2]);
        // r1 += COEFFICIENT4 * (float) (roller_skating_score[1] - roller_skating_score[0] - roller_skating_score[2]) / (1 + roller_skating_score[0] + roller_skating_score[1] + roller_skating_score[2]);
        // r2 += COEFFICIENT4 * (float) (roller_skating_score[2] - roller_skating_score[0] - roller_skating_score[1]) / (1 + roller_skating_score[0] + roller_skating_score[1] + roller_skating_score[2]);

        // r0 += COEFFICIENT4 * (float) (diving_score[0] - diving_score[1] - diving_score[2]) / (1 + diving_score[0] + diving_score[1] + diving_score[2]);
        // r1 += COEFFICIENT4 * (float) (diving_score[1] - diving_score[0] - diving_score[2]) / (1 + diving_score[0] + diving_score[1] + diving_score[2]);
        // r2 += COEFFICIENT4 * (float) (diving_score[2] - diving_score[0] - diving_score[1]) / (1 + diving_score[0] + diving_score[1] + diving_score[2]);

        // r0 += COEFFICIENT2 * ((score0 > score1 && score0 > score2) - (score0 < score1 && score0 < score2));
        // r1 += COEFFICIENT2 * ((score1 > score0 && score1 > score2) - (score1 < score0 && score1 < score2));
        // r2 += COEFFICIENT2 * ((score2 > score0 && score2 > score1) - (score2 < score0 && score2 < score1));

        // r0 += COEFFICIENT2 * (hurdle_race_score[0] - hurdle_race_score[1] - hurdle_race_score[2]);
        // r1 += COEFFICIENT2 * (hurdle_race_score[1] - hurdle_race_score[0] - hurdle_race_score[2]);
        // r2 += COEFFICIENT2 * (hurdle_race_score[2] - hurdle_race_score[0] - hurdle_race_score[1]);

        // r0 += COEFFICIENT2 * (archery_score[0] - archery_score[1] - archery_score[2]);
        // r1 += COEFFICIENT2 * (archery_score[1] - archery_score[0] - archery_score[2]);
        // r2 += COEFFICIENT2 * (archery_score[2] - archery_score[0] - archery_score[1]);

        // r0 += COEFFICIENT2 * (roller_skating_score[0] - roller_skating_score[1] - roller_skating_score[2]);
        // r1 += COEFFICIENT2 * (roller_skating_score[1] - roller_skating_score[0] - roller_skating_score[2]);
        // r2 += COEFFICIENT2 * (roller_skating_score[2] - roller_skating_score[0] - roller_skating_score[1]);

        // r0 += COEFFICIENT2 * (diving_score[0] - diving_score[1] - diving_score[2]);
        // r1 += COEFFICIENT2 * (diving_score[1] - diving_score[0] - diving_score[2]);
        // r2 += COEFFICIENT2 * (diving_score[2] - diving_score[0] - diving_score[1]);
        

        //     if (PLAYER_IDX == 0) {
        //         r0 = (float) (score0 - score1 - score2) / sum; // + (score0 > score1 && score0 > score2) - (score0 < score1 && score0 < score2);
        //         r1 = (float) (score1 - 2 * score0 - score2) / sum; // + (score1 > score0 && score1 > score2) - (score1 < score0 && score1 < score2);
        //         r2 = (float) (score2 - 2 * score0 - score1) / sum; // + (score2 > score0 && score2 > score1) - (score2 < score0 && score2 < score1);
        //     }
        //     else
        //     if (PLAYER_IDX == 1) {
        //         r0 = (float) (score0 - 2 * score1 - score2) / sum; // + (score0 > score1 && score0 > score2) - (score0 < score1 && score0 < score2);
        //         r1 = (float) (score1 - score0 - score2) / sum; // + (score1 > score0 && score1 > score2) - (score1 < score0 && score1 < score2);
        //         r2 = (float) (score2 - score0 - 2 * score1) / sum; // + (score2 > score0 && score2 > score1) - (score2 < score0 && score2 < score1);
        //     }
        //     else {
        //         r0 = (float) (score0 - score1 - 2 * score2) / sum; // + (score0 > score1 && score0 > score2) - (score0 < score1 && score0 < score2);
        //         r1 = (float) (score1 - score0 - 2 * score2) / sum; // + (score1 > score0 && score1 > score2) - (score1 < score0 && score1 < score2);
        //         r2 = (float) (score2 - score0 - score1) / sum; // + (score2 > score0 && score2 > score1) - (score2 < score0 && score2 < score1);
        //     }
        // }

        // losing -> attack the weakest link (secret strategy: dont care about the winning guy)
        // if (PLAYER_IDX == 0 && score0 < score1 && score0 < score2) {
        //     r0 = (float) (score0 - std::min(score1, score2)) / (score0 + std::min(score1, score2));
        // }
        // else
        // if (PLAYER_IDX == 1 && score1 < score0 && score1 < score2) {
        //     r1 = (float) (score1 - std::min(score0, score2)) / (score1 + std::min(score0, score2));
        // }
        // else
        // if (PLAYER_IDX == 2 && score2 < score0 && score2 < score1) {
        //     r2 = (float) (score2 - std::min(score0, score1)) / (score2 + std::min(score0, score1));
        // }

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

            hurdle_race_left = 1;
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

            if (hurdle_race.expected_end() <= 5) {
                hurdle_race_left = 1;
                std::cerr << "Hurdle once more\n";
            }
            else
            if (!hurdle_race.playable(0) &&
                !hurdle_race.playable(1) &&
                !hurdle_race.playable(2)) {    
                hurdle_race.end = true;
            }
            else {
                hurdle_race_left = 0; // 1;
            }
        }

        if (gpu[1] == "GAME_OVER") {
            archery.end = true;

            archery_left = 1; // 1;
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

            // archery.build_dp();

            if (archery.expected_end() <= 3) {
                archery_left = 1;
                std::cerr << "Archery once more\n";
            }
            // else
            // if (!archery.playable(0) &&
            //     !archery.playable(1) &&
            //     !archery.playable(2)) {
            //     archery.end = true;
            // }
            else {
                archery_left = 0; // 1;
            }
        }

        if (gpu[2] == "GAME_OVER") {
            roller_skating.end = true;

            roller_skating_left = 1;
        }
        else { // roller_skating
            roller_skating.order = 0;
            for (int i = 0; i < 4; i++) {
                roller_skating.order |= uint8_t(i) << (to_move_index(gpu[2][i]) << 1);
            }

            for (int i = 0; i < 3; i++) {
                roller_skating.dist_div10[i] = reg[2][i] / 10;
                roller_skating.dist_mod10[i] = reg[2][i] % 10;

                roller_skating.risk[i] = reg[2][i + 3];
            }

            roller_skating.turns_left = reg[2][6];
            // roller_skating.turns_done = 0;

            roller_skating.end = false;

            if (roller_skating.expected_end() <= 4) {
                roller_skating_left = 1;
                std::cerr << "Skating once more\n";
            }
            else
            if (!roller_skating.playable(0) &&
                !roller_skating.playable(1) &&
                !roller_skating.playable(2)) {
                roller_skating.end = true;
            }
            else {
                roller_skating_left = 0; // 1;
            }
        }

        if (gpu[3] == "GAME_OVER") {
            diving.end = true;
            diving_left = 0; // 0 because this game is totally random;
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
            

            // if (diving.expected_end() <= 3) {
            //     diving_left = 0;
            //     std::cerr << "Diving once more\n";
            // }
            // else
            if (!diving.playable(0) &&
                !diving.playable(1) &&
                !diving.playable(2)) {
                diving.end = true;
            }
            // else {
                diving_left = 0; // 1;
            // }
        }
    }
    
    inline bool still_playing() const {
        return !hurdle_race.end ||
               !archery.end ||
               !roller_skating.end ||
               !diving.end;
    }

    // inline void greedy_moves(int8_t* move) const {
    //     for (int i = 0; i < 3; i++) {

    //         uint8_t moves = hurdle_race.greedy_moves(i) |
    //                         archery.greedy_moves(i) |
    //                         roller_skating.greedy_moves(i) |
    //                         diving.greedy_moves(i) | 
    //                         (uint8_t(1) << (fast_rand() & 3));
            
    //         uint8_t order = all_permutations[fast_rand() % 24];

    //         move[i] = -1;
    //         for (int j = 0; j < 4; j++) {
    //             if ((moves >> (order & 3)) & 1) {
    //                 move[i] = j;
    //                 break;
    //             }

    //             order >>= 2;
    //         }

    //         // assert(0 <= move[i] && move[i] <= 3);

    //         // // std::cerr << "player: " << i << '\n';

    //         // int eval[4] = {0, 0, 0, 0};

    //         // if (hurdle_race.playable(i)) {
    //         //     uint8_t good_moves = hurdle_race.greedy_moves(i);
    //         //     for (int j = 0; j < 4; j++) {
    //         //         if (good_moves & (1 << j)) {
    //         //             eval[j] += int(archery_score[i]) * roller_skating_score[i] * diving_score[i] + 1;
    //         //         }
    //         //     }
    //         // }
    //         // // else {
    //         // //     std::cerr << "hurdles useless\n";
    //         // // }

    //         // if (archery.playable(i)) {
    //         //     uint8_t good_moves = archery.greedy_moves(i);
    //         //     for (int j = 0; j < 4; j++) {
    //         //         if (good_moves & (1 << j)) {
    //         //             eval[j] += int(hurdle_race_score[i]) * roller_skating_score[i] * diving_score[i] + 1;
    //         //         }
    //         //     }
    //         // }
    //         // // else {
    //         // //     std::cerr << "archery useless\n";
    //         // // }

    //         // if (roller_skating.playable(i)) {
    //         //     uint8_t good_moves = roller_skating.greedy_moves(i);
    //         //     for (int j = 0; j < 4; j++) {
    //         //         if (good_moves & (1 << j)) {
    //         //             eval[j] += int(hurdle_race_score[i]) * archery_score[i] * diving_score[i] + 1;
    //         //         }
    //         //     }
    //         // }
    //         // // else {
    //         // //     std::cerr << "skating useless\n";
    //         // // }

    //         // if (diving.playable(i)) {
    //         //     uint8_t good_moves = diving.greedy_moves(i);
    //         //     for (int j = 0; j < 4; j++) {
    //         //         if (good_moves & (1 << j)) {
    //         //             eval[j] += int(hurdle_race_score[i]) * archery_score[i] * roller_skating_score[i] + 1;
    //         //         }
    //         //     }
    //         // }
    //         // // else {
    //         // //     std::cerr << "diving useless\n";
    //         // // }

    //         // // std::cerr << "eval: ";
    //         // // for (int j = 0; j < 4; j++) {
    //         //     // std::cerr << int(eval[j]) << ' ';
    //         // // }
    //         // // std::cerr << '\n';

    //         // int best_score = -1;
    //         // for (int j = 0; j < 4; j++) {
    //         //     if (best_score < eval[j]) {
    //         //         best_score = eval[j];
    //         //         move[i] = j;
    //         //     }
    //         //     else
    //         //     if (best_score == eval[j] && fast_rand() & 1) {
    //         //         best_score = eval[j];
    //         //         move[i] = j;
    //         //     }
    //         // }
    //     }
    // }

    // void play_greedy() {
    //     int8_t move[3];

    //     greedy_moves(move);
    //     play(move[0], move[1], move[2]);
    // }

    void play_random() {
        const int8_t move[3] = {random_move(), random_move(), random_move()};

        hurdle_race.play(move);
        archery.play(move);
        roller_skating.play(move);
        diving.play(move);

        turn++;
    }

    void play(int8_t p0, int8_t p1, int8_t p2) {

        const int8_t move[3] = {p0, p1, p2};

        static float places[3];
        bool was_hurdle_race_end = hurdle_race.end;
        bool was_archery_end = archery.end;
        bool was_roller_skating_end = roller_skating.end;
        bool was_diving_end = diving.end;

        hurdle_race.play(move);
        archery.play(move);
        roller_skating.play(move);
        diving.play(move);

        turn++;

        if (was_hurdle_race_end && hurdle_race_left) {
            hurdle_race.generate_places(places);
            for (int i = 0; i < 3; i++) {
                hurdle_race_score[i] += places[i];
            }

            hurdle_race.randomize();
            hurdle_race_left--;
        }

        if (was_archery_end && archery_left) {
            archery.generate_places(places);
            for (int i = 0; i < 3; i++) {
                archery_score[i] += places[i];
            }

            archery.randomize();
            archery_left--;
        }

        if (was_roller_skating_end && roller_skating_left) {
            roller_skating.generate_places(places);
            for (int i = 0; i < 3; i++) {
                roller_skating_score[i] += places[i];
            }

            roller_skating.randomize();
            roller_skating_left--;
        }

        if (was_diving_end && diving_left) {
            diving.generate_places(places);
            for (int i = 0; i < 3; i++) {
                diving_score[i] += places[i];
            }

            diving.randomize();
            diving_left--;
        }
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
