#ifndef HURDLE_RACE
#define HURDLE_RACE

#include "../const.hpp"

#include <iostream>
#include <cassert>

struct HurdleRace {
    
    static int dp[TRACK_LENGTH];
    static int8_t dp_opt[TRACK_LENGTH];

    uint32_t track;

    int8_t pos[3], stun[3];
    bool end;

    void debug() const {
        std::cerr << "TRACK: ";
        for (int i = 0; i < TRACK_LENGTH; i++) {
            if (track & (1u << i))
                std::cerr << '#';
            else
                std::cerr << '.';
        }
        std::cerr << "|\n";

        for (int i = 0; i < 3; i++) {
            std::cerr << "i: " << i << " => " << int(pos[i]) << ' ' << int(stun[i]) << '\n';
        }
    }

    void generate_places(int8_t* places) const {
        for (int i = 0; i < 3; i++) {
            if (pos[i] >= TRACK_LENGTH - 1) {
                places[i] = 3;
            }
            else
            if (pos[i] < pos[(i + 1) % 3] && pos[i] < pos[(i + 2) % 3]) {
                places[i] = 0;
            }
            else {
                places[i] = 1;
            }
        }
    }

    inline void play(const int8_t* move) {
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
                    if (track & (1U << pos[i])) {
                        stun[i] = 2;
                    }
                    else {
                        pos[i]++;
                    }
                }
                else {
                    pos[i]++;
                    if (track & (1U << pos[i])) {
                        stun[i] = 2;
                    }
                    else {
                        pos[i]++;
                        if (track & (1U << pos[i])) {
                            stun[i] = 2;
                        }
                        else {
                            pos[i]++;
                        }
                    }
                }

                if (track & (1U << pos[i])) {
                    stun[i] = 2;
                }
                else
                if (pos[i] >= TRACK_LENGTH - 1) {
                    end = true;
                }
            }
        }
    }

    bool in_waiting(const int8_t player_idx) const {
        return stun[player_idx] > 0;
    }

    void build_dp() {
        dp[TRACK_LENGTH - 1] = 0;

        for (int i = TRACK_LENGTH - 2; i >= 0; i--) {
            
        }
    }

    int8_t greedy_move(const int8_t player_idx) const {
        return dp_opt[pos[player_idx]];
    }
};

int HurdleRace::dp[TRACK_LENGTH];
int8_t dp_opt[TRACK_LENGTH];

#endif // HURDLE_RACE