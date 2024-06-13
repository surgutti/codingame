#ifndef STATE_HPP
#define STATE_HPP

#include "minigames/hurdle_race.hpp"
#include "minigames/archery.hpp"
#include "minigames/roller_skating.hpp"
#include "minigames/diving.hpp"

#include <algorithm>

struct State {

    HurdleRace hurdle_race;
    int hurdle_race_score;

    Archery archery;
    int archery_score;

    // RollerSkating roller_skating;
    // int roller_skating_score;

    Diving diving;
    int diving_score;

    // p0 is the player; p1,p2 are enemies
    int evaluate() const {
        int score_sum = hurdle_race_score +
                        archery_score +
                        diving_score + 1;

        int score = 0;

        if (hurdle_race.end) {
            // penalty for not being 1st
            // reward for being 1st

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
            int mult = (archery_score == 0 ? 5 : 1);

            if (archery.places[0] == 1) {
                score += mult * 3;
            }
            else
            if (archery.places[0] == 2) {
                score += mult * 1;
            }
            else {
                score -= mult * 3;
            }
        }
        else {
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

        }
        else {
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