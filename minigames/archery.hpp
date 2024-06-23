#ifndef ARCHERY_HPP
#define ARCHERY_HPP

#include "../const.hpp"
#include "../random.hpp"

#include <vector>
#include <iostream>

struct Archery {

    static int dp[ARCHERY_LENGTH + 1][41][41];
    static uint8_t dp_opt[ARCHERY_LENGTH + 1][41][41];

    int8_t wind[ARCHERY_LENGTH];
    int8_t wind_index;

    int8_t x[3];
    int8_t y[3];

    bool end;

    void build_dp() const {
        for (int xx = -20; xx <= +20; xx++) {
            for (int yy = -20; yy <= +20; yy++) {
                dp[0][xx + 20][yy + 20] = xx * xx + yy * yy;
            }
        }

        for (int i = 1; i <= ARCHERY_LENGTH; i++) {
            for (int xx = -20; xx <= +20; xx++) {
                for (int yy = -20; yy <= +20; yy++) {
                    int& val = dp[i][xx + 20][yy + 20];
                    uint8_t& opt = dp_opt[i][xx + 20][yy + 20];

                    val = 100000;
                    opt = 0;

                    static constexpr int8_t dx[4] = {0, -1, 0, +1};
                    static constexpr int8_t dy[4] = {-1, 0, +1, 0};
                    
                    const int8_t wind_strength = wind[i - 1];

                    for (int move = 0; move < 4; move++) {
                        int xxx = xx + wind_strength * dx[move];
                        int yyy = yy + wind_strength * dy[move];

                        if (xxx > +20)  xxx = +20;
                        else
                        if (xxx < -20)  xxx = -20;
                    
                        if (yyy > +20)  yyy = +20;
                        else
                        if (yyy < -20)  yyy = -20;

                        int new_val = dp[i - 1][xxx + 20][yyy + 20];

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

    bool is_useless(int8_t player_idx) const {
        return false;
    }

    uint8_t greedy_moves(int8_t player_idx) const {
        if (wind_index > 7) {
            return 0b1111;
        }

        return dp_opt[wind_index + 1][x[player_idx] + 20][y[player_idx] + 20];
    }

    bool operator== (const Archery &other) const {
        if (end != other.end)
            return false;

        if (end)
            return true;

        for (int i = 0; i <= wind_index; i++)
            if (wind[i] != other.wind[i])
                return false;

        for (int i = 0; i < 3; i++) {
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
        for (int i = 0; i < 3; i++) {
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

        for (int i = 0; i < 3; i++) {
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

        int16_t score[3];
        for (int i = 0; i < 3; i++) {
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

        static constexpr int8_t dx[4] = {0, -1, 0, +1};
        static constexpr int8_t dy[4] = {-1, 0, +1, 0};

        const int8_t wind_strength = wind[wind_index];

        for (int i = 0; i < 3; i++) {
            x[i] += wind_strength * dx[move[i]];
            y[i] += wind_strength * dy[move[i]];

            if (x[i] > +20) x[i] = +20;
            else
            if (x[i] < -20) x[i] = -20;
        
            if (y[i] > +20) y[i] = +20;
            else
            if (y[i] < -20) y[i] = -20;
        }

        if (wind_index == 0) {
            end = true;
        }
        else {
            wind_index--;
        }
    }
};

int Archery::dp[ARCHERY_LENGTH + 1][41][41];
uint8_t Archery::dp_opt[ARCHERY_LENGTH + 1][41][41];

#endif // ARCHERY_HPP