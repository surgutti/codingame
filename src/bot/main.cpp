#ifndef LOCAL
    #undef _GLIBCXX_DEBUG
    #pragma GCC optimize("Ofast,inline")
    #pragma GCC target("bmi,bmi2,lzcnt,popcnt")
    #pragma GCC target("movbe")
    #pragma GCC target("aes,pclmul,rdrnd")
    #pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2")
#endif // LOCAL

#include "core/const.hpp"
#include "state/state.hpp"
#include "search/brain.hpp"

#include <array>
#include <iostream>
#include <string>

int TURN;
int NB_GAMES;
constexpr bool kEnableLogs = false;

constexpr int scaled_timeout(int base_timeout_ms) {
    const int scaled = (base_timeout_ms * TIMEOUT_SCALE_PCT) / 100;
    return (scaled > 0 ? scaled : 1);
}

constexpr const char* move_list[MOVE_COUNT] = {
    "UP", "LEFT", "DOWN", "RIGHT"
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    if constexpr (kEnableLogs) {
        std::cerr << "BrainNode: " << sizeof(BrainNode) << '\n';
        std::cerr << "State: " << sizeof(State) << '\n';
    }

    std::cin >> PLAYER_IDX;
    std::cin >> NB_GAMES;

    if (NB_GAMES != GAME_COUNT) {
        return 0;
    }

    timer.start();

    // Precompute static hurdle DP tables once.
    Hurdles::build_dp();

    State::InputGpuArray gpu{};
    State::InputRegArray reg{};
    Brain brain;
    for (TURN = 0; ; TURN++) {
        State current_state;
        
        int final_score[PLAYER_COUNT];
        for (int i = 0; i < PLAYER_COUNT; i++) {
            std::cin >> final_score[i];

            {
                int gold, silver, bronze;
                std::cin >> gold >> silver >> bronze;

                current_state.hurdles_score[i] = 3 * gold + silver;
            }
            {
                int gold, silver, bronze;
                std::cin >> gold >> silver >> bronze;

                current_state.archery_score[i] = 3 * gold + silver;
            }
            {
                int gold, silver, bronze;
                std::cin >> gold >> silver >> bronze;

                current_state.skating_score[i] = 3 * gold + silver;
            }
            {
                int gold, silver, bronze;
                std::cin >> gold >> silver >> bronze;

                current_state.divings_score[i] = 3 * gold + silver;
            }
        }

        if (TURN != 0) {
            timer.start();
        }

        // Parse the 4 minigame GPU strings + register arrays.
        for (int i = 0; i < GAME_COUNT; i++) {
            std::cin >> gpu[i];

            for (int j = 0; j < REG_FIELDS; j++) {
                std::cin >> reg[i][j];
            }
        }
        current_state.init(gpu, reg);

        if constexpr (kEnableLogs) {
            std::cerr << "Brain START\n";
        }

        current_state.turn = TURN;

#ifdef PSYLEAGUE
        brain.run(current_state, scaled_timeout(TIMEOUT_PSYLEAGUE_MS));
#else
        brain.run(current_state, scaled_timeout(TURN == 0 ? TIMEOUT_TURN0_MS : TIMEOUT_TURN_MS));
#endif // PSYLEAGUE

        if constexpr (kEnableLogs) {
            std::cerr << "timer: " << timer.get_elapsed() << '\n';
            std::cerr << "brain: " << brain.roots[0]->vis << '\n';
            std::cerr << "pool: " << (float) BrainNode::last / BRAIN_POOL << '\n';
            std::cerr << "last: " << BrainNode::last << '\n';
        }

        int move = brain.best_move(PLAYER_IDX);

        if constexpr (kEnableLogs) {
            std::cerr << "predicting: \n";
            std::cerr << "0: " << move_list[brain.best_move(0)] << '\n';
            std::cerr << "1: " << move_list[brain.best_move(1)] << '\n';
            std::cerr << "2: " << move_list[brain.best_move(2)] << '\n';
        }

        std::cout << move_list[move] << '\n' << std::flush;
        if constexpr (kEnableLogs) {
            std::cerr << '\n';
        }
    }

}
