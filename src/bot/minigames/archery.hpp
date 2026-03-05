#ifndef ARCHERY_HPP
#define ARCHERY_HPP

#include "../core/const.hpp"
#include "../core/random.hpp"

#include <iostream>

struct Archery {

    static int dp[ARCHERY_LENGTH + 1][ARCHERY_GRID_SIZE][ARCHERY_GRID_SIZE];
    static uint8_t dp_opt[ARCHERY_LENGTH + 1][ARCHERY_GRID_SIZE][ARCHERY_GRID_SIZE];
    static uint16_t transitions[10][ARCHERY_GRID_SIZE][ARCHERY_GRID_SIZE][MOVE_COUNT];
    static bool transitions_ready;

    int8_t wind[ARCHERY_LENGTH];
    int8_t wind_index;

    int8_t x[PLAYER_COUNT];
    int8_t y[PLAYER_COUNT];

    bool end;

    static inline int clamp_coord(int value) {
        if (value > ARCHERY_GRID_RADIUS) {
            return ARCHERY_GRID_RADIUS;
        }
        if (value < -ARCHERY_GRID_RADIUS) {
            return -ARCHERY_GRID_RADIUS;
        }
        return value;
    }

    static inline void ensure_transitions() {
        if (transitions_ready) {
            return;
        }

        static constexpr int8_t dx[MOVE_COUNT] = {0, -1, 0, +1};
        static constexpr int8_t dy[MOVE_COUNT] = {-1, 0, +1, 0};

        for (int wind_strength = 0; wind_strength <= 9; wind_strength++) {
            for (int xx = -ARCHERY_GRID_RADIUS; xx <= ARCHERY_GRID_RADIUS; xx++) {
                for (int yy = -ARCHERY_GRID_RADIUS; yy <= ARCHERY_GRID_RADIUS; yy++) {
                    const int xx_index = xx + ARCHERY_GRID_RADIUS;
                    const int yy_index = yy + ARCHERY_GRID_RADIUS;

                    for (int move = 0; move < MOVE_COUNT; move++) {
                        const int next_x = clamp_coord(xx + wind_strength * dx[move]);
                        const int next_y = clamp_coord(yy + wind_strength * dy[move]);

                        transitions[wind_strength][xx_index][yy_index][move] =
                            uint16_t(next_x + ARCHERY_GRID_RADIUS) * ARCHERY_GRID_SIZE +
                            uint16_t(next_y + ARCHERY_GRID_RADIUS);
                    }
                }
            }
        }

        transitions_ready = true;
    }

    void build_dp() const {
        ensure_transitions();

        for (int xx = -ARCHERY_GRID_RADIUS; xx <= ARCHERY_GRID_RADIUS; xx++) {
            for (int yy = -ARCHERY_GRID_RADIUS; yy <= ARCHERY_GRID_RADIUS; yy++) {
                dp[0][xx + ARCHERY_GRID_RADIUS][yy + ARCHERY_GRID_RADIUS] = xx * xx + yy * yy;
            }
        }

        for (int i = 1; i <= ARCHERY_LENGTH; i++) {
            const int8_t wind_strength = wind[i - 1];
            const int* prev_layer = &dp[i - 1][0][0];

            for (int xx = 0; xx < ARCHERY_GRID_SIZE; xx++) {
                for (int yy = 0; yy < ARCHERY_GRID_SIZE; yy++) {
                    int& val = dp[i][xx][yy];
                    uint8_t& opt = dp_opt[i][xx][yy];

                    val = 100000;
                    opt = 0;

                    const uint16_t* next = transitions[wind_strength][xx][yy];

                    for (int move = 0; move < MOVE_COUNT; move++) {
                        const int new_val = prev_layer[next[move]];

                        if (val > new_val) {
                            val = new_val;
                            opt = uint8_t(1) << move;
                        }
                        else
                        if (val == new_val) {
                            opt |= uint8_t(1) << move;
                        }
                    }
                }
            }
        }
    }

    bool is_useless([[maybe_unused]] int8_t player_idx) const {
        return false;
    }

    uint8_t greedy_moves(int8_t player_idx) const {
        if (wind_index > 7) {
            return 0b1111;
        }

        return dp_opt[wind_index + 1][x[player_idx] + ARCHERY_GRID_RADIUS][y[player_idx] + ARCHERY_GRID_RADIUS];
    }

    bool operator== (const Archery &other) const {
        if (end != other.end)
            return false;

        if (end)
            return true;

        for (int i = 0; i <= wind_index; i++)
            if (wind[i] != other.wind[i])
                return false;

        for (int i = 0; i < PLAYER_COUNT; i++) {
            if (x[i] != other.x[i])
                return false;
            if (y[i] != other.y[i])
                return false;
        }

        return wind_index == other.wind_index;
    }

    void debug() const {
        std::cerr << "wind_index: " << wind_index << '\n';
        std::cerr << "wind: ";
        for (int i = wind_index; i >= 0; i--) {
            std::cerr << int(wind[i]) << ' ';
        }
        std::cerr << '\n';
        for (int i = 0; i < PLAYER_COUNT; i++) {
            std::cerr << "i: " << i << " => " << int(x[i]) << ' ' << int(y[i]) << '\n';
        }
    }

    inline int expected_end() const {
        return wind_index + 1;
    }

    void randomize() {
        int8_t sx = 5 + fast_rand() % 5;
        int8_t sy = 5 + fast_rand() % 5;

        if (fast_rand() & 1)
            sx = -sx;
        
        if (fast_rand() & 1)
            sy = -sy;
        
        wind_index = 12 + fast_rand() % 4 - 1;

        for (int i = 0; i < PLAYER_COUNT; i++) {
            x[i] = sx;
            y[i] = sy;
        }

        constexpr float weights[10] = { 0, 2, 2, 2, 0.5, 0.5, 0.25, 0.25, 0.25, 0.2 };
        constexpr float sum = weights[0] + weights[1] + weights[2] + 
                              weights[3] + weights[4] + weights[5] + 
                              weights[6] + weights[7] + weights[8] + 
                              weights[9];

        for (int i = 0; i <= wind_index; i++) {
            wind[i] = 9;

            float w = (fast_rand() % 1000) * sum;

            for (int j = 1; j < 9; j++) {
                w -= weights[j] * 1000;

                if (w < 0) {
                    wind[i] = j;
                    break;
                }
            }
        }

        end = false;
    }

    inline void generate_places(float* places) const {

        int16_t score[PLAYER_COUNT];
        for (int i = 0; i < PLAYER_COUNT; i++) {
            score[i] = (int16_t) x[i] * x[i] + (int16_t) y[i] * y[i]; 
        }

        if (score[0] <= score[1] && score[0] <= score[2]) {
            places[0] = 3;
        }
        else
        if (score[0] > score[1] && score[0] > score[2]) {
            places[0] = 0;
        }
        else {
            places[0] = 1;
        }

        if (score[1] <= score[0] && score[1] <= score[2]) {
            places[1] = 3;
        }
        else
        if (score[1] > score[0] && score[1] > score[2]) {
            places[1] = 0;
        }
        else {
            places[1] = 1;
        }

        if (score[2] <= score[0] && score[2] <= score[1]) {
            places[2] = 3;
        }
        else
        if (score[2] > score[0] && score[2] > score[1]) {
            places[2] = 0;
        }
        else {
            places[2] = 1;
        }
    }

    inline void play(const int8_t* move) {
        if (end) {
            return;
        }

        static constexpr int8_t dx[MOVE_COUNT] = {0, -1, 0, +1};
        static constexpr int8_t dy[MOVE_COUNT] = {-1, 0, +1, 0};

        const int8_t wind_strength = wind[wind_index];

        for (int i = 0; i < PLAYER_COUNT; i++) {
            x[i] += wind_strength * dx[move[i]];
            y[i] += wind_strength * dy[move[i]];

            if (x[i] > +ARCHERY_GRID_RADIUS) x[i] = +ARCHERY_GRID_RADIUS;
            else
            if (x[i] < -ARCHERY_GRID_RADIUS) x[i] = -ARCHERY_GRID_RADIUS;
        
            if (y[i] > +ARCHERY_GRID_RADIUS) y[i] = +ARCHERY_GRID_RADIUS;
            else
            if (y[i] < -ARCHERY_GRID_RADIUS) y[i] = -ARCHERY_GRID_RADIUS;
        }

        if (wind_index == 0) {
            end = true;
        }
        else {
            wind_index--;
        }
    }
};

int Archery::dp[ARCHERY_LENGTH + 1][ARCHERY_GRID_SIZE][ARCHERY_GRID_SIZE];
uint8_t Archery::dp_opt[ARCHERY_LENGTH + 1][ARCHERY_GRID_SIZE][ARCHERY_GRID_SIZE];
uint16_t Archery::transitions[10][ARCHERY_GRID_SIZE][ARCHERY_GRID_SIZE][MOVE_COUNT];
bool Archery::transitions_ready = false;

#endif // ARCHERY_HPP
