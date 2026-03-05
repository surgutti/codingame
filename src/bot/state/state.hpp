#ifndef STATE_HPP
#define STATE_HPP

#include "../minigames/hurdles.hpp"
#include "../minigames/archery.hpp"
#include "../minigames/skating.hpp"
#include "../minigames/divings.hpp"

#include "../core/utils.hpp"
#include "../core/const.hpp"

#include <array>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

struct State {

    Hurdles hurdles;
    Archery archery;
    Skating skating;
    Divings divings;

    float   hurdles_score[PLAYER_COUNT];
    float   archery_score[PLAYER_COUNT];
    float   skating_score[PLAYER_COUNT];
    float   divings_score[PLAYER_COUNT];

    int8_t  hurdles_left;
    int8_t  archery_left;
    int8_t  skating_left;
    int8_t  divings_left;

    int8_t turn;

    using InputGpuArray = std::array<std::string, GAME_COUNT>;
    using InputRegArray = std::array<std::array<int, REG_FIELDS>, GAME_COUNT>;

    static inline void add_mask_weight(float* weights, uint8_t moves, float weight) {
        weights[0] += (moves & 1) ? weight : 0.0f;
        weights[1] += (moves & 2) ? weight : 0.0f;
        weights[2] += (moves & 4) ? weight : 0.0f;
        weights[3] += (moves & 8) ? weight : 0.0f;
    }

    static inline float move_count_bonus(uint8_t moves) {
        return 1.0f / float(1 + pop_count(moves));
    }

    inline void apply_places() {
        // Add medal points for every minigame that is currently finished.
        float places[PLAYER_COUNT] = {0, 0, 0};

        if (hurdles.end) {
            hurdles.generate_places(places);
            for (int i = 0; i < PLAYER_COUNT; i++) {
                hurdles_score[i] += places[i];
            }
        }

        if (archery.end) {
            archery.generate_places(places);
            for (int i = 0; i < PLAYER_COUNT; i++) {
                archery_score[i] += places[i];
            }
        }

        if (skating.end) {
            skating.generate_places(places);
            for (int i = 0; i < PLAYER_COUNT; i++) {
                skating_score[i] += places[i];
            }
        }

        if (divings.end) {
            divings.generate_places(places);
            for (int i = 0; i < PLAYER_COUNT; i++) {
                divings_score[i] += places[i];
            }
        }
    }

    inline bool is_terminal() const {
        return (hurdles.end && hurdles_left == 0 &&
                archery.end && archery_left == 0 &&
                skating.end && skating_left == 0 &&
                divings.end && divings_left == 0) ||
                turn >= MAX_TURNS;
    }

    inline bool is_rollout_terminal() const {
        return (hurdles.end &&
                archery.end &&
                skating.end &&
                divings.end) ||
                turn >= MAX_TURNS;
    }

    inline void get_stats(float* rewards) {

        apply_places();

        // Convert medal-like partial scores into multiplicative game scores.
        float scores[PLAYER_COUNT] = {1, 1, 1};

        for (int i = 0; i < PLAYER_COUNT; i++) { 
            if (hurdles_score[i] > 1)
                scores[i] *= hurdles_score[i];

            if (archery_score[i] > 1)
                scores[i] *= archery_score[i];
            
            if (skating_score[i] > 1)
                scores[i] *= skating_score[i];
            
            if (divings_score[i] > 1)
                scores[i] *= divings_score[i];
        }

        {
            float max_enemy = std::max<float>(scores[1], scores[2]);
            float min_enemy = std::min<float>(scores[1], scores[2]);

            rewards[0] = float(scores[0] - 0.55 * max_enemy - 1.0 * min_enemy) / (scores[0] + 0.55 * max_enemy + 1.0 * min_enemy);
        }

        {
            float max_enemy = std::max<float>(scores[0], scores[2]);
            float min_enemy = std::min<float>(scores[0], scores[2]);

            rewards[1] = float(scores[1] - 0.55 * max_enemy - 1.0 * min_enemy) / (scores[1] + 0.55 * max_enemy + 1.0 * min_enemy);
        }

        {
            float max_enemy = std::max<float>(scores[0], scores[1]);
            float min_enemy = std::min<float>(scores[0], scores[1]);

            rewards[2] = float(scores[2] - 0.55 * max_enemy - 1.0 * min_enemy) / (scores[2] + 0.55 * max_enemy + 1.0 * min_enemy);
        }
    }

    void init(const InputGpuArray& gpu, const InputRegArray& reg) {

        // Decode CodinGame input frame into internal minigame states.

        if (gpu[0] == "GAME_OVER") {
            hurdles.end = true;
            hurdles_left = 1;
        }
        else {
            hurdles.track = 0;
            for (int i = 0; i < (int) gpu[0].size(); i++) {
                if (gpu[0][i] == '#') {
                    hurdles.track |= uint32_t(1) << i;
                }
            }

            for (int i = 0; i < PLAYER_COUNT; i++) {
                hurdles.pos[i] = reg[0][i];
                hurdles.stun[i] = reg[0][i + 3];
            }

            hurdles.end = false;

            hurdles.find_track_index();

            if (hurdles.expected_end() <= 7) {
                hurdles_left = 1;
            }
            else
            if (!hurdles.playable(0) &&
                !hurdles.playable(1) &&
                !hurdles.playable(2)) {
                hurdles.end = true;
            }
            else {
                hurdles_left = 0;
            }
        }

        if (gpu[1] == "GAME_OVER") {
            archery.end = true;
            archery_left = 1;
        }
        else {
            archery.wind_index = (int) gpu[1].size() - 1;
            
            for (int i = 0; i < (int) gpu[1].size(); i++) {
                archery.wind[archery.wind_index - i] = int(gpu[1][i] - '0');
            }

            for (int i = 0; i < PLAYER_COUNT; i++) {
                archery.x[i] = reg[1][2 * i + 0];
                archery.y[i] = reg[1][2 * i + 1];
            }

            archery.build_dp();

            archery.end = false;
            if (archery.expected_end() <= 2) {
                archery_left = 1;
            }
            else {
                archery_left = 0;
            }
        }

        if (gpu[2] == "GAME_OVER") {
            skating.end = true;
            skating_left = 1;
            skating.turns_left = 7;
        }
        else {
            skating.order = 0;
            for (int i = 0; i < MOVE_COUNT; i++) {
                skating.order |= uint8_t(i) << (to_move_index(gpu[2][i]) << 1);
            }

            for (int i = 0; i < PLAYER_COUNT; i++) {
                skating.dist_div10[i] = reg[2][i] / 10;
                skating.dist_mod10[i] = reg[2][i] % 10;

                skating.risk[i] = reg[2][i + 3];
            }

            skating.turns_left = std::min<int>(5, reg[2][6]);

            skating.end = false;

            if (skating.expected_end() <= 4) {
                skating_left = 1;
            }
            else {
                skating_left = 0;
            }
        }

        if (gpu[3] == "GAME_OVER") {
            divings.end = true;
            divings_left = 0; // this game is totally random
        }
        else {
            divings.goals_left = (int8_t) gpu[3].size();

            divings.goal = 0;
            for (int i = 0; i < (int) gpu[3].size(); i++) {
                divings.goal |= uint32_t(to_move_index(gpu[3][i])) << (2 * i);
            }
            
            for (int i = 0; i < PLAYER_COUNT; i++) {
                divings.score[i] = reg[3][i];
                divings.combo[i] = reg[3][i + 3];
            }

            divings.end = false;

            if (!divings.playable(0) &&
                !divings.playable(1) &&
                !divings.playable(2)) {
                divings.end = true;
            }

            divings_left = 0;
        }
    }

    void init(const std::vector<std::string>& gpu,
              const std::vector<std::vector<int>>& reg) {
        InputGpuArray gpu_arr{};
        InputRegArray reg_arr{};

        for (int i = 0; i < GAME_COUNT; i++) {
            gpu_arr[i] = gpu[i];
            for (int j = 0; j < REG_FIELDS; j++) {
                reg_arr[i][j] = reg[i][j];
            }
        }

        init(gpu_arr, reg_arr);
    }

    int8_t greedy_move(int player_idx) const {
        if constexpr (ROLLOUT_GREEDY_RANDOM_PCT > 0) {
            if (fast_rand() % 100 < ROLLOUT_GREEDY_RANDOM_PCT) {
                return random_move();
            }
        }

        float weights[MOVE_COUNT] = {
            GREEDY_BASE_WEIGHTS[0],
            GREEDY_BASE_WEIGHTS[1],
            GREEDY_BASE_WEIGHTS[2],
            GREEDY_BASE_WEIGHTS[3]
        };

        if (!hurdles.is_useless(player_idx)) {
            uint8_t moves = hurdles.greedy_moves(player_idx);
            float weight = 0.0f;
            if constexpr (WEIGHT_MODE == 1) {
                weight = 1.0f / (1.0f + hurdles_score[player_idx]) + move_count_bonus(moves);
            }
            else {
                weight = (archery_score[player_idx] + 1.0f) *
                         (skating_score[player_idx] + 1.0f) *
                         (divings_score[player_idx] + 1.0f) + move_count_bonus(moves);
            }

            add_mask_weight(weights, moves, weight);
        }

        if (!archery.is_useless(player_idx)) {
            uint8_t moves = archery.greedy_moves(player_idx);
            float weight = 0.0f;
            if constexpr (WEIGHT_MODE == 1) {
                weight = 1.0f / (1.0f + archery_score[player_idx]) + move_count_bonus(moves);
            }
            else {
                weight = (hurdles_score[player_idx] + 1.0f) *
                         (skating_score[player_idx] + 1.0f) *
                         (divings_score[player_idx] + 1.0f) + move_count_bonus(moves);
            }

            add_mask_weight(weights, moves, weight);
        }
        
        // if (!skating.is_useless(player_idx)) {
        //     uint8_t moves = skating.greedy_moves(player_idx);
        //     const float weight = (hurdles_score[player_idx] + 1) *
        //                          (archery_score[player_idx] + 1) *
        //                          (divings_score[player_idx] + 1) + 1.0f / (1 + pop_count(moves));

        //     // std::cerr << "Skating: " << weight << '\n';
        //     for (int i = 0; i < 4; i++) {
        //         if (moves & 1) {
        //             weights[i] += weight;
        //         }
        //         moves >>= 1;
        //     }
        // }
        
        if (!divings.is_useless(player_idx)) {
            uint8_t moves = divings.greedy_moves(player_idx);
            float weight = 0.0f;
            if constexpr (WEIGHT_MODE == 1) {
                weight = 1.0f / (1.0f + divings_score[player_idx]) + move_count_bonus(moves);
            }
            else {
                weight = (hurdles_score[player_idx] + 1.0f) *
                         (archery_score[player_idx] + 1.0f) *
                         (skating_score[player_idx] + 1.0f);
            }

            add_mask_weight(weights, moves, weight);
        }

        float weights_sum = weights[0] + weights[1] + weights[2] + weights[3];

        // std::cerr << "weights: ";
        // for (int i = 0; i < 4; i++) {
        //     std::cerr << weights[i] << ' ';
        // }
        // std::cerr << '\n';

        int p = fast_rand() % int(100 * weights_sum);

        for (int i = 0; i < MOVE_COUNT - 1; i++) {
            p -= 100 * weights[i];

            if (p < 0) {
                // std::cerr << "> " << i << '\n';
                return i;
            }
        }
        // std::cerr << "> " << 3 << '\n';

        return 3;
    }

    void play_greedy() {
        const int8_t move[PLAYER_COUNT] = {greedy_move(0), greedy_move(1), greedy_move(2)};

        hurdles.play(move);
        archery.play(move);
        skating.play(move);
        divings.play(move);

        turn++;
    }

    void play_random() {
        play(random_move(), random_move(), random_move());
    }

    void play(int8_t p0, int8_t p1, int8_t p2) {

        const int8_t move[PLAYER_COUNT] = {p0, p1, p2};

        bool was_hurdles_end = hurdles.end;
        bool was_archery_end = archery.end;
        bool was_skating_end = skating.end;
        bool was_divings_end = divings.end;

        hurdles.play(move);
        archery.play(move);
        skating.play(move);
        divings.play(move);

        turn++;

        if constexpr (REGENERATE_ENDED_GAMES) {
            float places[PLAYER_COUNT];

            if (was_hurdles_end && hurdles_left) {
                hurdles.generate_places(places);
                for (int i = 0; i < PLAYER_COUNT; i++) {
                    hurdles_score[i] += places[i];
                }

                // Optional extra minigame pass configured by *_left flags.
                hurdles.randomize();
                hurdles_left--;
            }

            if (was_archery_end && archery_left) {
                archery.generate_places(places);
                for (int i = 0; i < PLAYER_COUNT; i++) {
                    archery_score[i] += places[i];
                }

                archery.randomize();
                archery_left--;
            }

            if (was_skating_end && skating_left) {
                skating.generate_places(places);
                for (int i = 0; i < PLAYER_COUNT; i++) {
                    skating_score[i] += places[i];
                }

                skating.randomize();
                skating_left--;
            }

            if (was_divings_end && divings_left) {
                divings.generate_places(places);
                for (int i = 0; i < PLAYER_COUNT; i++) {
                    divings_score[i] += places[i];
                }

                divings.randomize();
                divings_left--;
            }
        }
        else {
            if (was_hurdles_end) {
                hurdles_left = 0;
            }
            if (was_archery_end) {
                archery_left = 0;
            }
            if (was_skating_end) {
                skating_left = 0;
            }
            if (was_divings_end) {
                divings_left = 0;
            }
        }
    }

    void debug() const {
        std::cerr << "state: " << hurdles.end << ' ' << archery.end << ' ' << skating.end << ' ' << divings.end << '\n';
        hurdles.debug();
        archery.debug();
        skating.debug();
        divings.debug();
    }

};

#endif // STATE_HPP
