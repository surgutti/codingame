#ifndef DIVING_HPP
#define DIVING_HPP

#include "../const.hpp"
#include "../random.hpp"

#include <algorithm>
#include <vector>
#include <utility>

struct Diving {

    int32_t goal;
    int8_t goals_left;

    uint8_t score[3];
    int8_t combo[3];

    bool end;

    bool operator== (const Diving &other) const {
        if (end != other.end)
            return false;
        
        if (end)
            return true;

        for (int i = 0; i < 3; i++) {
            if (score[i] != other.score[i])
                return false;
            
            if (combo[i] != other.combo[i])
                return false;
        }

        return goal == other.goal;
    }

    void debug() const {
        std::cerr << "goals_left: " << int(goals_left) << '\n';
        std::cerr << "goal: ";
        for (int i = 0; i < goals_left; i++) {
            std::cerr << int((goal >> (i * 2)) & 3) << ' ';
        }
        std::cerr << '\n';
        for (int i = 0; i < 3; i++) {
            std::cerr << "i: " << i << " => " << int(score[i]) << ' ' << int(combo[i]) << '\n';
        }
    }

    inline int expected_end() const {
        return goals_left;
    }

    inline void generate_places(float* places) const {
        if (end) {
            if (score[0] >= score[1] && score[0] >= score[2]) {
                places[0] = 3;
            }
            else
            if (score[0] < score[1] && score[0] < score[2]) {
                places[0] = 0;
            }
            else {
                places[0] = 1;
            }

            if (score[1] >= score[0] && score[1] >= score[2]) {
                places[1] = 3;
            }
            else
            if (score[1] < score[0] && score[1] < score[2]) {
                places[1] = 0;
            }
            else {
                places[1] = 1;
            }

            if (score[2] >= score[0] && score[2] >= score[1]) {
                places[2] = 3;
            }
            else
            if (score[2] < score[0] && score[2] < score[1]) {
                places[2] = 0;
            }
            else {
                places[2] = 1;
            }
        }
        else {
            float fst_p0 = fst_dp[goals_left][id[score[0]][combo[0]]][id[score[1]][combo[1]]] *
                           fst_dp[goals_left][id[score[0]][combo[0]]][id[score[2]][combo[2]]];

            float trd_p0 = snd_dp[goals_left][id[score[0]][combo[0]]][id[score[1]][combo[1]]] *
                           snd_dp[goals_left][id[score[0]][combo[0]]][id[score[2]][combo[2]]];
        
            float snd_p0 = 1.0 - fst_p0 - trd_p0;

            places[0] = 3 * fst_p0 + 1 * snd_p0;


            float fst_p1 = fst_dp[goals_left][id[score[1]][combo[1]]][id[score[0]][combo[0]]] *
                           fst_dp[goals_left][id[score[1]][combo[1]]][id[score[2]][combo[2]]];

            float trd_p1 = snd_dp[goals_left][id[score[1]][combo[1]]][id[score[0]][combo[0]]] *
                           snd_dp[goals_left][id[score[1]][combo[1]]][id[score[2]][combo[2]]];
        
            float snd_p1 = 1.0 - fst_p1 - trd_p1;

            places[1] = 3 * fst_p1 + 1 * snd_p1;

            float fst_p2 = fst_dp[goals_left][id[score[2]][combo[2]]][id[score[0]][combo[0]]] *
                           fst_dp[goals_left][id[score[2]][combo[2]]][id[score[1]][combo[1]]];

            float trd_p2 = snd_dp[goals_left][id[score[2]][combo[2]]][id[score[0]][combo[0]]] *
                           snd_dp[goals_left][id[score[2]][combo[2]]][id[score[1]][combo[1]]];
        
            float snd_p2 = 1.0 - fst_p2 - trd_p2;

            places[2] = 3 * fst_p2 + 1 * snd_p2;
        }
    }

    void randomize() {
        goals_left = 15; // 12 + fast_rand() % 4;
        goal = fast_rand(); // dont care about the rest? -> just slowing down

        for (int i = 0; i < 3; i++) {
            score[i] = 0;
            combo[i] = 0;
        }

        end = false;
    }

    inline void play(const int8_t* move) {
        if (end) {
            return;
        }
        
        for (int i = 0; i < 3; i++) {
            if (move[i] == (goal & 3)) {
                combo[i]++;
                score[i] += combo[i];
            }
            else {
                combo[i] = 0;
            }
        }

        if (goals_left == 1) {
            end = true;
        }
        else {
            goal >>= 2;
            goals_left--;
        }
    }

    bool in_waiting(const int8_t player_idx) const {
        return false;
    }

    inline bool playable(int8_t player_idx) const {
        const int8_t enemy1_idx = (player_idx + 1) % 3;
        const int8_t enemy2_idx = (player_idx + 2) % 3;

        int best_player = score[player_idx] + combo[player_idx] * goals_left + ((int(goals_left) * (goals_left + 1)) >> 1);
        int worst_player = score[player_idx];

        int best_enemy1 = score[enemy1_idx] + combo[enemy1_idx] * goals_left + ((int(goals_left) * (goals_left + 1)) >> 1);
        int worst_enemy1 = score[enemy1_idx];

        int best_enemy2 = score[enemy2_idx] + combo[enemy2_idx] * goals_left + ((int(goals_left) * (goals_left + 1)) >> 1);
        int worst_enemy2 = score[enemy2_idx];

        if (worst_player >= best_enemy1 && worst_player >= best_enemy2) {
            return false; // inevitable 1st place
        }

        if (best_player < worst_enemy1 && best_player < worst_enemy2) {
            return false; // inevitable 3rd place
        }

        if (worst_player >= best_enemy1 && best_player < worst_enemy2) {
            return false; // inevitable 2nd place
        }

        if (best_player < worst_enemy1 && worst_player >= best_enemy2) {
            return false; // inevitable 2nd place
        }

        // assert (int(goals_left) * (goals_left + 1) < 256);
        // if (score[player_idx] >= score[enemy1_idx] + combo[enemy1_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1) &&
        //     score[player_idx] >= score[enemy2_idx] + combo[enemy2_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1)) {
        //     return false; // inevitable 1st place
        // }

        // assert (int(goals_left) * (goals_left + 1) < 256);
        // if (score[player_idx] + combo[player_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1) < score[enemy1_idx] &&
        //     score[player_idx] + combo[player_idx] * goals_left + ((uint8_t(goals_left) * (goals_left + 1)) >> 1) < score[enemy2_idx]) {
        //     return false; // inevitable 3rd place
        // }

        return true;
    }

    inline uint8_t greedy_moves(const int8_t player_idx) const {
        if (end)
            return 0;
        
        return uint8_t(1) << (goal & 3);
    }

    static float fst_dp[16][341][341];
    static float snd_dp[16][341][341];
    static int   id[140][20];

    static void build_dp() {
        std::vector<std::pair<int, int>> all;
        
        all.emplace_back(0, 0);
        for (int i = 1; i <= 15; i++) {
            std::vector<std::pair<int, int>> nxt = all;

            for (auto [a, b] : all) {
                
                nxt.emplace_back(a, 0);
                nxt.emplace_back(a + b + 1, b + 1);
            }

            std::sort(nxt.begin(), nxt.end());

            nxt.erase(std::unique(nxt.begin(), nxt.end()), nxt.end());
            all = nxt;
        }

        const int possible = (int) all.size();

        // std::cerr << "possible: " << possible << '\n';
        assert(possible == 341);

        for (int i = 0; i < possible; i++) {
            auto [a, b] = all[i];
            id[a][b] = i;
        }

        for (int i = 0; i < possible; i++) {
            for (int j = 0; j < possible; j++) {
                if (all[i].first >= all[j].first) {
                    fst_dp[0][i][j] = 1.0f;
                }

                if (all[i].first < all[j].first) {
                    snd_dp[0][i][j] = 1.0f;
                }
            }
        }

        for (int k = 1; k <= 15; k++) {
            for (int i = 0; i < possible; i++) {
                auto [score_0, combo_0] = all[i];

                int ii = id[score_0 + combo_0 + 1][combo_0 + 1];

                if (ii == 0) {
                    std::cerr << "> " << score_0 + combo_0 + 1 << ' ' << combo_0 + 1 << '\n';
                    std::cerr << "> " << score_0 << ' ' << combo_0 << '\n';
                    std::cerr << " id: " << id[score_0][combo_0] << '\n';
                    continue;
                }

                for (int j = 0; j < possible; j++) {
                    auto [score_1, combo_1] = all[j];

                    int jj = id[score_1 + combo_1 + 1][combo_1 + 1];

                    if (jj == 0)
                        continue;

                    fst_dp[k][i][j] = fst_dp[k - 1][i][j]   * 0.75 * 0.75 +
                                      fst_dp[k - 1][ii][j]  * 0.25 * 0.75 +
                                      fst_dp[k - 1][i][jj]  * 0.75 * 0.25 +
                                      fst_dp[k - 1][ii][jj] * 0.25 * 0.25;

                    snd_dp[k][i][j] = snd_dp[k - 1][i][j]   * 0.75 * 0.75 +
                                      snd_dp[k - 1][ii][j]  * 0.25 * 0.75 +
                                      snd_dp[k - 1][i][jj]  * 0.75 * 0.25 +
                                      snd_dp[k - 1][ii][jj] * 0.25 * 0.25;
                }
            }
        }
    }
};

float Diving::fst_dp[16][341][341];
float Diving::snd_dp[16][341][341];
int   Diving::id[140][20];

#endif // DIVING_HPP