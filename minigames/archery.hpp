#ifndef ARCHERY_HPP
#define ARCHERY_HPP

#include "../const.hpp"

struct Archery {

    int wind[ARCHERY_LENGTH];
    int wind_index;

    int x[3];
    int y[3];

    bool end;

    int scores[3];
    int places[3];

    void debug() const {
        std::cerr << "wind_index: " << wind_index << '\n';
        std::cerr << "wind: ";
        for (int i = wind_index; i >= 0; i--) {
            std::cerr << wind[i] << ' ';
        }
        std::cerr << '\n';
        for (int i = 0; i < 3; i++) {
            std::cerr << "i: " << i << " => " << x[i] << ' ' << y[i] << '\n';
        }
    }

    void play(const int* move) {
        if (end) {
            return;
        }

        static const int dx[4] = {0, -1, 0, +1};
        static const int dy[4] = {-1, 0, +1, 0};

        for (int i = 0; i < 3; i++) {
            x[i] += wind[wind_index] * dx[move[i]];
            y[i] += wind[wind_index] * dy[move[i]];
        
            if (x[i] < -20) x[i] = -20;
            if (x[i] > +20) x[i] = +20;

            if (y[i] < -20) y[i] = -20;
            if (y[i] > +20) y[i] = +20;
        }

        if (wind_index == 0) {
            for (int i = 0; i < 3; i++) {
                scores[i] = x[i] * x[i] + y[i] * y[i]; 
            }

            for (int i = 0; i < 3; i++) {
                if (scores[i] <= scores[(i + 1) % 3] && scores[i] <= scores[(i + 2) % 3]) {
                    places[i] = 1;
                }
                else
                if (scores[i] > scores[(i + 1) % 3] && scores[i] > scores[(i + 2) % 3]) {
                    places[i] = 3;
                }
                else {
                    places[i] = 2;
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