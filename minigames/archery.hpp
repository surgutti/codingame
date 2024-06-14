#ifndef ARCHERY_HPP
#define ARCHERY_HPP

#include "../const.hpp"

struct Archery {

    int8_t wind[ARCHERY_LENGTH];
    int8_t wind_index;

    int8_t x[3];
    int8_t y[3];

    bool end;

    int8_t places[3];

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

    void play(const int8_t* move) {
        if (end) {
            return;
        }

        static constexpr int8_t dx[4] = {0, -1, 0, +1};
        static constexpr int8_t dy[4] = {-1, 0, +1, 0};

        const int8_t wind_strength = wind[wind_index];

        for (int i = 0; i < 3; i++) {
            x[i] += wind_strength * dx[move[i]];

            if (x[i] > +20) x[i] = +20;
            else
            if (x[i] < -20) x[i] = -20;
            
            y[i] += wind_strength * dy[move[i]];
        
            if (y[i] > +20) y[i] = +20;
            else
            if (y[i] < -20) y[i] = -20;
        }

        if (wind_index == 0) {

            int16_t scores[3];
            for (int i = 0; i < 3; i++) {
                scores[i] = (int16_t) x[i] * x[i] + (int16_t) y[i] * y[i]; 
            }

            for (int i = 0; i < 3; i++) {
                if (scores[i] <= scores[(i + 1) % 3] && scores[i] <= scores[(i + 2) % 3]) {
                    places[i] = 3;
                }
                else
                if (scores[i] > scores[(i + 1) % 3] && scores[i] > scores[(i + 2) % 3]) {
                    places[i] = 0;
                }
                else {
                    places[i] = 1;
                }
            }

            end = true;
        }
        else {
            wind_index--;
        }
    }
    
    bool proven_win(int p_idx) const {
        return false;
    }

    bool proven_lost(int p_idx) const {
        return false;
    }

};

#endif // ARCHERY_HPP