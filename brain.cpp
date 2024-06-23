#ifndef LOCAL
    #undef _GLIBCXX_DEBUG
    #pragma GCC optimize("Ofast,inline")
    #pragma GCC target("bmi,bmi2,lzcnt,popcnt")
    #pragma GCC target("movbe")
    #pragma GCC target("aes,pclmul,rdrnd")
    #pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2")
#endif // LOCAL

#include "const.hpp"
#include "state.hpp"
#include "brain.hpp"

#include <iostream>
#include <string>
#include <vector>

int TURN;
int NB_GAMES;

const std::vector<std::string> move_list = {
    "UP", "LEFT", "DOWN", "RIGHT"
};

int main() {
    
    std::cerr << "BrainNode: " << sizeof(BrainNode) << '\n';
    std::cerr << "State: " << sizeof(State) << '\n';

    std::cin >> PLAYER_IDX;
    std::cin.ignore();
    std::cin >> NB_GAMES;
    std::cin.ignore();

    timer.start();

    Hurdles::build_dp();

    Brain brain;
    for (TURN = 0; ; TURN++) {
        State current_state;
        
        int final_score[3];
        for (int i = 0; i < 3; i++) {
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

        std::cerr << "Brain START\n";

        current_state.turn = TURN;

#ifdef PSYLEAGUE
        brain.run(current_state, 20);
#else
        brain.run(current_state, (TURN == 0 ? 600 : 45));
#endif // PSYLEAGUE

        std::cerr << "timer: " << timer.get_elapsed() << '\n';
        std::cerr << "brain: " << brain.roots[0]->vis << '\n';
        std::cerr << "pool: " << (float) BrainNode::last / BRAIN_POOL << '\n';
        std::cerr << "last: " << BrainNode::last << '\n';

        int move = brain.best_move(PLAYER_IDX);

        std::cerr << "predicting: \n";
        std::cerr << "0: " << move_list[brain.best_move(0)] << '\n';
        std::cerr << "1: " << move_list[brain.best_move(1)] << '\n';
        std::cerr << "2: " << move_list[brain.best_move(2)] << '\n';

        std::cout << move_list[move] << std::endl;
        std::cerr << '\n';
    }

}
