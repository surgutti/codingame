#ifndef ARCHERY_HPP
#define ARCHERY_HPP

#include "../const.hpp"
#include "../random.hpp"

#include <vector>

struct Archery {

    static int dp[ARCHERY_LENGTH][41][41];
    static int pd[ARCHERY_LENGTH][41][41];
    static uint8_t dp_opt[ARCHERY_LENGTH][41][41];

    int8_t wind[ARCHERY_LENGTH];
    int8_t wind_index;

    int8_t x[3];
    int8_t y[3];

    bool end;

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

    inline void generate_places(int8_t* places) const {
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

    void build_dp() {
        for (int xxx = -20; xxx <= +20; xxx++) {
            for (int yyy = -20; yyy <= +20; yyy++) {
                static constexpr int8_t dx[4] = {0, -1, 0, +1};
                static constexpr int8_t dy[4] = {-1, 0, +1, 0};

                int wind_strength = wind[0];

                int& value = dp[0][20 + xxx][20 + yyy];
                int& worst = pd[0][20 + xxx][20 + yyy];
                uint8_t& opt = dp_opt[0][20 + xxx][20 + yyy];

                opt = 0;
                value = 10000;
                worst = 0;
                for (int8_t move = 0; move < 4; move++) {
                    int xx = xxx + wind_strength * dx[move];
                    int yy = yyy + wind_strength * dy[move];

                    if (xx < -20)   xx = -20;
                    if (xx > +20)   xx = +20;
                    if (yy < -20)   yy = -20;
                    if (yy > +20)   yy = +20;

                    int now = xx * xx + yy * yy;

                    if (value > now) {
                        value = now;
                        opt = 1 << move;
                    }
                    else
                    if (value == now) {
                        opt |= 1 << move;
                    }

                    if (worst < now) {
                        worst = now;
                    }
                }

            }
        }

        for (int i = 1; i <= wind_index; i++) {
            for (int xxx = -20; xxx <= +20; xxx++) {
                for (int yyy = -20; yyy <= +20; yyy++) {

                    static constexpr int8_t dx[4] = {0, -1, 0, +1};
                    static constexpr int8_t dy[4] = {-1, 0, +1, 0};

                    int wind_strength = wind[i];

                    int& value = dp[i][20 + xxx][20 + yyy];
                    int& worst = pd[i][20 + xxx][20 + yyy];

                    uint8_t& opt = dp_opt[i][20 + xxx][20 + yyy];
                    
                    opt = 0;
                    value = 10000;
                    worst = 0;
                    for (int8_t move = 0; move < 4; move++) {
                        int xx = xxx + wind_strength * dx[move];
                        int yy = yyy + wind_strength * dy[move];

                        if (xx < -20)   xx = -20;
                        if (xx > +20)   xx = +20;
                        if (yy < -20)   yy = -20;
                        if (yy > +20)   yy = +20;

                        int now = dp[i - 1][xx + 20][yy + 20];

                        if (value > now) {
                            value = now;
                            opt = 1 << move;
                        }
                        else
                        if (value == now) {
                            opt |= 1 << move;
                        }

                        if (worst < pd[i - 1][xx + 20][yy + 20]) {
                            worst = pd[i - 1][xx + 20][yy + 20];
                        }
                    }
                }
            }
        }
    }

    bool in_waiting(const int8_t player_idx) const {
        return false;
    }

    inline bool playable(const int8_t player_idx) const {
        // too little enchancement to hope for early win

        const int8_t enemy1_idx = (player_idx + 1) % 3;
        const int8_t enemy2_idx = (player_idx + 2) % 3;

        if (pd[wind_index][x[player_idx] + 20][y[player_idx] + 20] <=
            dp[wind_index][x[enemy1_idx] + 20][y[enemy1_idx] + 20] &&

            pd[wind_index][x[player_idx] + 20][y[player_idx] + 20] <=
            dp[wind_index][x[enemy2_idx] + 20][y[enemy2_idx] + 20]) {
            return false; // inevitable 1st place
        }

        if (dp[wind_index][x[player_idx] + 20][y[player_idx] + 20] >
            pd[wind_index][x[enemy1_idx] + 20][y[enemy1_idx] + 20] &&
            
            dp[wind_index][x[player_idx] + 20][y[player_idx] + 20] >
            pd[wind_index][x[enemy2_idx] + 20][y[enemy2_idx] + 20]) {
            return false; // inevitable 3rd place
        }

        if (pd[wind_index][x[player_idx] + 20][y[player_idx] + 20] <=
            dp[wind_index][x[enemy1_idx] + 20][y[enemy1_idx] + 20] &&

            dp[wind_index][x[player_idx] + 20][y[player_idx] + 20] >
            pd[wind_index][x[enemy2_idx] + 20][y[enemy2_idx] + 20]) {
            return false; // inevitable 2nd place
        }

        if (pd[wind_index][x[player_idx] + 20][y[player_idx] + 20] <=
            dp[wind_index][x[enemy2_idx] + 20][y[enemy2_idx] + 20] &&
            
            dp[wind_index][x[player_idx] + 20][y[player_idx] + 20] >
            pd[wind_index][x[enemy1_idx] + 20][y[enemy1_idx] + 20]) {
            return false; // inevitable 2nd place
        }

        return true;
    }

    inline uint8_t greedy_moves(const int8_t player_idx) const {
        if (end)
            return 0;
        
        return dp_opt[wind_index][x[player_idx] + 20][y[player_idx] + 20];
    }
};

int Archery::dp[ARCHERY_LENGTH][41][41];
int Archery::pd[ARCHERY_LENGTH][41][41];
uint8_t Archery::dp_opt[ARCHERY_LENGTH][41][41];

#endif // ARCHERY_HPP