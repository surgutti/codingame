#ifndef HURDLE_RACE
#define HURDLE_RACE

#include "../const.hpp"

#include <iostream>
#include <cassert>

struct HurdleRace {
    char track[TRACK_LENGTH + 3];

    int pos[3], stun[3];

    bool end;

    int places[3];

    void debug() const {
        std::cerr << "TRACK: ";
        for (int i = 0; i < TRACK_LENGTH; i++) {
            std::cerr << track[i];
        }
        std::cerr << "|\n";

        for (int i = 0; i < 3; i++) {
            std::cerr << "i: " << i << " => " << pos[i] << ' ' << stun[i] << '\n';
        }
    }

    void play(const int* move) {
        if (end) {
            return;
        }

        for (int i = 0; i < 3; i++) {
            if (stun[i]) {
                stun[i]--;
            }
            else {
                if (move[i] == 1) {
                    pos[i]++;
                }
                else
                if (move[i] == 0) {
                    pos[i] += 2;
                }
                else
                if (move[i] == 2) {
                    pos[i]++;
                    if (track[pos[i]] == '#') {
                        stun[i] = 2;
                    }
                    else {
                        pos[i]++;
                    }
                }
                else
                if (move[i] == 3) {
                    pos[i]++;
                    if (track[pos[i]] == '#') {
                        stun[i] = 2;
                    }
                    else {
                        pos[i]++;
                        if (track[pos[i]] == '#') {
                            stun[i] = 2;
                        }
                        else {
                            pos[i]++;
                        }
                    }
                }
                else {
                    assert(false);
                }

                if (track[pos[i]] == '#') {
                    stun[i] = 2;
                }

                if (pos[i] >= TRACK_LENGTH) {
                    end = true;
                }
            }

            // std::cerr << "after: " << pos[i] << ' ' << stun[i] << '\n';
        }

        if (end) {
            for (int i = 0; i < 3; i++) {
                if (pos[i] >= TRACK_LENGTH) {
                    places[i] = 1;
                }
                else
                if (pos[i] < pos[(i + 1) % 3] && pos[i] < pos[(i + 2) % 3]) {
                    places[i] = 3;
                }
                else {
                    places[i] = 2;
                }
            }
        }
    }

    bool proven_win(int p_idx) const {
        return false;
    }

    bool proven_lost(int p_idx) const {
        return false;
    }
};

#endif // HURDLE_RACE