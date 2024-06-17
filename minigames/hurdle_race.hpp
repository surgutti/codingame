#ifndef HURDLE_RACE
#define HURDLE_RACE

#include "../const.hpp"

#include <iostream>
#include <cassert>

struct HurdleRace {
    
    static int dp[TRACK_LENGTH + 2]; // shortest time to finish
    static int pd[TRACK_LENGTH + 2]; // longest time to finish
    static uint8_t dp_opt[TRACK_LENGTH];

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
        dp[TRACK_LENGTH + 0] = 0;
        dp[TRACK_LENGTH + 1] = 0;

        dp_opt[TRACK_LENGTH - 1] = 0b1111;
        
        pd[TRACK_LENGTH - 1] = 0;
        pd[TRACK_LENGTH + 0] = 0;
        pd[TRACK_LENGTH + 1] = 0;

        for (int i = TRACK_LENGTH - 2; i >= 0; i--) {

            dp[i] = 1000;
            pd[i] = 0;
            dp_opt[i] = 0;
            for (int8_t move = 0; move < 4; move++) {
                int now = 0;

                int p = i;

                if (move == 1) {
                    p++;
                    now += 1;

                    if (track & (1U << p)) {
                        now += 2;
                    }
                }
                else
                if (move == 0) {
                    p += 2;
                    now += 1;
                    
                    if (track & (1U << p)) {
                        now += 2;
                    }
                }
                else
                if (move == 2) {
                    p++;
                    if (track & (1U << p)) {
                        now += 1 + 2;
                    }
                    else {
                        p++;
                        now += 1;

                        if (track & (1U << p)) {
                            now += 2;
                        }
                    }
                }
                else {
                    p++;
                    if (track & (1U << p)) {
                        now += 1 + 2;
                    }
                    else {
                        p++;
                        if (track & (1U << p)) {
                            now += 1 + 2;
                        }
                        else {
                            p++;
                            now += 1;

                            if (track & (1U << p)) {
                                now += 2;
                            }
                        }
                    }
                }

                if (dp[i] > now + dp[p]) {
                    dp[i] = now + dp[p];
                    dp_opt[i] = 1 << move;
                }
                else
                if (dp[i] == now + dp[p]) {
                    dp_opt[i] |= 1 << move;
                }

                if (pd[i] < now + pd[p]) {
                    pd[i] = now + pd[p]; // ! need to use the pd value, not the dp in now!!!
                }
            }
        }

        // std::cerr << "dp: ";
        // for (int i = 0; i < TRACK_LENGTH; i++) {
        //     std::cerr << int(dp[i]) << ' ';
        // }
        // std::cerr << '\n';

        // std::cerr << "dp_opt: ";
        // for (int i = 0; i < TRACK_LENGTH; i++) {
        //     std::cerr << int(dp_opt[i]) << ' ';
        // }
        // std::cerr << '\n';

        // std::cerr << "pd: ";
        // for (int i = 0; i < TRACK_LENGTH; i++) {
        //     std::cerr << int(pd[i]) << ' ';
        // }
        // std::cerr << '\n';
    }

    inline bool playable(const int8_t player_idx) const {
        // if (stun[player_idx]) {
        //     return false;
        // }

        const int8_t enemy1_idx = (player_idx + 1) % 3;
        const int8_t enemy2_idx = (player_idx + 2) % 3;

        if (pd[pos[player_idx]] + stun[player_idx] <=
            dp[pos[enemy1_idx]] + stun[enemy1_idx] &&
            
            pd[pos[player_idx]] + stun[player_idx] <=
            dp[pos[enemy2_idx]] + stun[enemy2_idx]) {
            return false; // inevitable 1st place
        }

        if (dp[pos[player_idx]] + stun[player_idx] >
            pd[pos[enemy1_idx]] + stun[enemy1_idx] &&

            dp[pos[player_idx]] + stun[player_idx] >
            pd[pos[enemy2_idx]] + stun[enemy2_idx]) {
            return false; // inevitable 3rd place
        }

        // ranking: <enemy1> <player> <enemy2>
        if (dp[pos[player_idx]] + stun[player_idx] >
            pd[pos[enemy1_idx]] + stun[enemy1_idx] &&
            
            pd[pos[player_idx]] + stun[player_idx] <=
            dp[pos[enemy2_idx]] + stun[enemy2_idx]) {
            return false; // inevitable 2nd place
        }

        // ranking: <enemy2> <player> <enemy1>
        if (dp[pos[player_idx]] + stun[player_idx] >
            pd[pos[enemy2_idx]] + stun[enemy2_idx] &&
            
            pd[pos[player_idx]] + stun[player_idx] <=
            dp[pos[enemy1_idx]] + stun[enemy1_idx]) {
            return false; // inevitable 2nd place
        }

        return true;
    }

    inline uint8_t greedy_moves(const int8_t player_idx) const {
        if (end || stun[player_idx])
            return 0;

        return dp_opt[pos[player_idx]];
    }
};

int HurdleRace::dp[TRACK_LENGTH + 2];
int HurdleRace::pd[TRACK_LENGTH + 2];
uint8_t HurdleRace::dp_opt[TRACK_LENGTH];

#endif // HURDLE_RACE