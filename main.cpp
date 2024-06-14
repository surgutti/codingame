#ifndef LOCAL
    #undef _GLIBCXX_DEBUG
    #pragma GCC optimize("Ofast,inline")
    #pragma GCC target("bmi,bmi2,lzcnt,popcnt")
    #pragma GCC target("movbe")
    #pragma GCC target("aes,pclmul,rdrnd")
    #pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2")
#endif // LOCAL

#include "state.hpp"
// #include "search.hpp"
#include "mcts.hpp"

#include <iostream>
#include <string>
#include <vector>

int TURN;
int PLAYER_IDX;
int NB_GAMES;

int main() {

    std::cerr << sizeof(State) << '\n';
    std::cerr << sizeof(bool) << '\n';

    std::cin >> PLAYER_IDX;
    std::cin.ignore();
    std::cin >> NB_GAMES;
    std::cin.ignore();

    timer.start();

    MCTS mcts;

    for (TURN = 0; ; TURN++) {
        State current_state;
        
        for (int i = 0; i < 3; i++) {
            int final_score;
            std::cin >> final_score;

            {
                int gold, silver, bronze;
                std::cin >> gold >> silver >> bronze;

                current_state.hurdle_race_score[i] = 3 * gold + silver;
            }
            {
                int gold, silver, bronze;
                std::cin >> gold >> silver >> bronze;

                current_state.archery_score[i] = 3 * gold + silver;
            }
            {
                int gold, silver, bronze;
                std::cin >> gold >> silver >> bronze;

                current_state.roller_skating_score[i] = 3 * gold + silver;
            }
            {
                int gold, silver, bronze;
                std::cin >> gold >> silver >> bronze;

                current_state.diving_score[i] = 3 * gold + silver;
            }
        }

        if (TURN != 0) {
            timer.start();
        }

        {
            std::vector<std::string> gpu(NB_GAMES);
            std::vector<std::vector<int>> reg(NB_GAMES);

            for (int i = 0; i < NB_GAMES; i++) {
                std::cin >> gpu[i];

                reg[i].resize(7);
                for (int j = 0; j < 7; j++) {
                    std::cin >> reg[i][j];
                }
                std::cin.ignore();
            }

            current_state.init(gpu, reg);
        }

        std::cerr << "MCTS START\n";

#ifdef PSYLEAGUE
        mcts.run(current_state, 45);
#else
        mcts.run(current_state, (TURN == 0 ? 950 : 45));
#endif // PSYLEAGUE

        mcts.debug();
        std::cerr << "timer: " << timer.get_elapsed() << '\n';
        std::cerr << "pool: " << (float) MCTSNode::last_node / MCTSNODE_POOL << '\n';
        std::cerr << "last: " << MCTSNode::last_node << '\n';

        int move = mcts.best_move(PLAYER_IDX);
        
        std::vector<std::string> move_list = {
            "UP", "LEFT", "DOWN", "RIGHT"
        };

        std::cout << move_list[move] << std::endl;

        return 0;
    }

}