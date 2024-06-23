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

int main() {
    
    std::cerr << "BrainNode: " << sizeof(BrainNode) << '\n';
    std::cerr << "State: " << sizeof(State) << '\n';
    
    timer.start();
    RollerSkating::build_dp();
    // std::cerr << "elapsed: " << timer.get_elapsed() << '\n';
    
    // timer.start();
    Diving::build_dp();
    // std::cerr << "elapsed: " << timer.get_elapsed() << '\n';

    // timer.start();
    HurdleRace::build_fst_snd_dp();
    std::cerr << "elapsed: " << timer.get_elapsed() << '\n';

    std::cerr << "Hurdle :\n";
    for (int i = 0; i < 29; i++) {
        std::cerr << "i: " << i << ' ';
        std::cerr << HurdleRace::fst_dp[0][i][0][i][0] << ' ';
        std::cerr << HurdleRace::snd_dp[0][i][0][i][0] << '\n';
    }

    std::cerr << "Skating:\n";
    for (int i = 0; i <= 15; i++) {
        std::cerr << "i: " << i << ' ';
        std::cerr << RollerSkating::fst_dp[i][0][2][0][2] << ' ';
        std::cerr << RollerSkating::snd_dp[i][0][2][0][2] << '\n';
    }

    std::cerr << "Diving:\n";
    for (int i = 0; i <= 15; i++) {
        std::cerr << "i: " << i << ' ';
        std::cerr << Diving::fst_dp[i][0][0] << ' ';
        std::cerr << Diving::snd_dp[i][0][0] << '\n';
    }

    std::cin >> PLAYER_IDX;
    std::cin.ignore();
    std::cin >> NB_GAMES;
    std::cin.ignore();


    timer.start();

    Brain brain;
    for (TURN = 0; ; TURN++) {
        State current_state;
        
        int final_score[3];
        for (int i = 0; i < 3; i++) {
            std::cin >> final_score[i];

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

        std::cerr << "Brain START\n";

        current_state.turn = TURN;

#ifdef PSYLEAGUE
        if (TURN == 0) { // to cancel init computations
            timer.start();
        }
        brain.run(current_state, 20);
        // brain.debug();
#else
        brain.run(current_state, (TURN == 0 ? 600 : 45));
        // brain.run(current_state, 60000);
        // brain.debug();
#endif // PSYLEAGUE

        // timer.start();

        // for (int i = 0; i < MCTSNode::last_node; i++) {
        //     for (int k = 0; k < 3; k++)
        //         for (int j = 0; j < 4; j++)
        //             MCTSNode::pool[i].vis[k][j] >>= 1;
        //     MCTSNode::pool[i].node_vis >>= 1;
        // }

        /*
        if i'm lossing -> attack the lowest link
        if i'm winning -> maximize the gap between me and second place
        if i'm second ->
        */
        std::cerr << "timer: " << timer.get_elapsed() << '\n';
        std::cerr << "pool: " << (float) BrainNode::last / BRAIN_POOL << '\n';
        std::cerr << "last: " << BrainNode::last << '\n';

        std::cerr << "DEPTH: ";
        for (int i = 0; i < 100; i++) {
            if (DEPTH[i])
                std::cerr << DEPTH[i] << ' ';
        }
        std::cerr << '\n';

        // int8_t greedy_moves[3];
        // current_state.greedy_moves(greedy_moves);
        // int move = greedy_moves[PLAYER_IDX];
        // std::cerr << "GREEDY: ";
        // for (int i = 0; i < 3; i++) {
        //     std::cerr << move_list[greedy_moves[i]] << ' ';
        // }
        // std::cerr << '\n';

        int move = brain.best_move(PLAYER_IDX);
        
        std::vector<std::string> move_list = {
            "UP", "LEFT", "DOWN", "RIGHT"
        };

        std::cerr << "predicting: \n";
        std::cerr << "0: " << move_list[brain.best_move(0)] << '\n';
        std::cerr << "1: " << move_list[brain.best_move(1)] << '\n';
        std::cerr << "2: " << move_list[brain.best_move(2)] << '\n';

        std::cerr << "BRAIN_PLAY: " << BRAIN_PLAY << '\n';
        std::cerr << "ROLLOUT_PLAY: " << ROLLOUT_PLAY << '\n';
        
        BRAIN_PLAY = 0;
        ROLLOUT_PLAY = 0;


        std::cout << move_list[move] << std::endl;

        // if (TURN == 1) {
        //     return 0;
        // }
    }

}
