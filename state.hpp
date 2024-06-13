#ifndef STATE_HPP
#define STATE_HPP

#include "minigames/hurdle_race.hpp"
#include "minigames/archery.hpp"
#include "minigames/roller_skating.hpp"
#include "minigames/diving.hpp"

#include <algorithm>
#include <vector>
#include <string>

struct State {

    HurdleRace hurdle_race;
    int hurdle_race_score;

    Archery archery;
    int archery_score;

    // RollerSkating roller_skating;
    int roller_skating_score;

    Diving diving;
    int diving_score;

    bool is_terminal() const {
        return hurdle_race.end ||
               archery.end ||
               diving.end;
    }

    // return how much does a player earn from games
    void get_stats(int r1, int r2, int r3) const {

    }

    void init(const std::vector<std::string>& gpu,
              const std::vector<std::vector<int>> reg,
              int player_idx) {
        int enemy1_idx = (player_idx + 1) % 3;
        int enemy2_idx = (player_idx + 2) % 3;

        { // hurdle_race
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

        { // archery
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

        { // roller_skating

        }

        { // diving
            diving.goal_index = (int) gpu[3].size() - 1;
            for (int i = 0; i < (int) gpu[3].size(); i++) {
                diving.goal[diving.goal_index - i] = to_move_index(gpu[3][i]);
            }
        }
    }

    /* p0 is the player; p1,p2 are enemies
        TODO: 
            -> score normalization for each of minigames (make them equaly important in some sense)
                (expecting: some value of game normalized between [-1000, +1000] * (sum of other games))
            -> big reward for winning
            -> some reward for second place
            -> penalty for losing
            -> for rollerskates just give reward for bigger jump and penalty for big risk in first move
    */
    int evaluate() const {

        int score = 0;

        if (hurdle_race.end) {
            int score_sum = archery_score +
                            roller_skating_score +
                            diving_score + 1;

            int mult = (hurdle_race_score == 0 ? 5 : 1);

            if (hurdle_race.places[0] == 1) {
                score += mult * 3 * score_sum;
            }
            if (hurdle_race.places[0] == 2) {
                score += mult * 1 * score_sum;
            }
            else {
                score -= mult * 3 * score_sum;
            }
        }
        else {
            int score_sum = archery_score +
                            roller_skating_score +
                            diving_score + 1;

            int mult = (hurdle_race_score == 0 ? 5 : 1);

            if (hurdle_race.proven_win(0)) {
                score += mult * 3 * score_sum;             
            }
            else
            if (hurdle_race.proven_lost(0)) {
                score -= mult * 3 * score_sum;
            }
            else {
                score += hurdle_race.pos[0];
                score -= hurdle_race.pos[1];
                score -= hurdle_race.pos[2];
            
                score -= hurdle_race.stun[0] * 2;
                score += hurdle_race.stun[1] * 2;
                score += hurdle_race.stun[2] * 2;
            }
        }

        if (archery.end) {
            int score_sum = hurdle_race_score +
                            roller_skating_score +
                            diving_score + 1;

            int mult = (archery_score == 0 ? 5 : 1);

            if (archery.places[0] == 1) {
                score += mult * 3 * score_sum;
            }
            else
            if (archery.places[0] == 2) {
                score += mult * 1 * score_sum;
            }
            else {
                score -= mult * 3 * score_sum;
            }
        }
        else {
            int score_sum = hurdle_race_score +
                            roller_skating_score +
                            diving_score + 1;

            int mult = (archery_score == 0 ? 5 : 1);

            if (archery.proven_win(0)) {
                score += mult * 3 * score_sum;             
            }
            else
            if (archery.proven_lost(0)) {
                score -= mult * 3 * score_sum;
            }
            else {
                int dist_0 = archery.x[0] * archery.x[0] + archery.y[0] * archery.y[0];
                int dist_1 = archery.x[1] * archery.x[1] + archery.y[1] * archery.y[1];
                int dist_2 = archery.x[2] * archery.x[2] + archery.y[2] * archery.y[2];

                int best_dist = std::min({dist_0, dist_1, dist_2});
                int worst_dist = std::min({dist_0, dist_1, dist_2});
                int median_dist = dist_0 ^ dist_1 ^ dist_2 ^ best_dist ^ worst_dist;

                score -= dist_0 - best_dist;
                score += (worst_dist - dist_0) * 2;
                score += median_dist - dist_0;
            }
        }

        // if (roller_skating.end) {

        // }
        // else {

        // }

        if (diving.end) {
            int score_sum = hurdle_race_score +
                            archery_score +
                            roller_skating_score + 1;

            int mult = (diving_score == 0 ? 5 : 1);

            if (diving.places[0] == 1) {
                score += mult * 3 * score_sum;
            }
            else
            if (diving.places[0] == 2) {
                score += mult * 1 * score_sum;
            }
            else {
                score -= mult * 3 * score_sum;
            }
        }
        else {
            int score_sum = hurdle_race_score +
                            archery_score +
                            roller_skating_score + 1;

            int mult = (diving_score == 0 ? 5 : 1);

            if (diving.proven_win(0)) {
                score += mult * 3 * score_sum;             
            }
            else
            if (diving.proven_lost(0)) {
                score -= mult * 3 * score_sum;
            }
            else {
                score += diving.score[0];
                score -= diving.score[1];
                score -= diving.score[2];

                score += diving.combo[0];
                score -= diving.combo[1];
                score -= diving.combo[2];

                int best_combo = std::max({diving.combo[0], diving.combo[1], diving.combo[2]});
                int best_score = std::max({diving.score[0], diving.score[1], diving.score[2]});

                int worst_combo = std::min({diving.combo[0], diving.combo[1], diving.combo[2]});
                int worst_score = std::min({diving.score[0], diving.score[1], diving.score[2]});

                int median_combo = diving.combo[0] ^ diving.combo[1] ^ diving.combo[2] ^ best_combo ^ worst_combo;
                int median_score = diving.score[0] ^ diving.score[1] ^ diving.score[2] ^ best_score ^ worst_score;

                score -= best_score - diving.score[0];
                score += (diving.score[0] - worst_score) * 2;
                score += diving.score[0] - median_score;

                score -= best_combo - diving.combo[0];
                score += (diving.combo[0] - worst_combo) * 2;
                score += diving.combo[0] - median_combo;
            }
        }

        return score;
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