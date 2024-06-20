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
// #include "search.hpp"
#include "mcts.hpp"

#include <iostream>
#include <string>
#include <vector>

int TURN;
int NB_GAMES;

int main() {
    
    std::cerr << "MCTSNode: " << sizeof(MCTSNode) << '\n';
    std::cerr << "State: " << sizeof(State) << '\n';

    std::cin >> PLAYER_IDX;
    std::cin.ignore();
    std::cin >> NB_GAMES;
    std::cin.ignore();

    timer.start();

    MCTS mcts;

    mcts.reset();

    State last_state;

    int8_t last_move;

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

        std::cerr << "MCTS START\n";

        current_state.turn = TURN;


        const std::vector<std::string> move_list = {
            "UP", "LEFT", "DOWN", "RIGHT"
        };


        // if (MCTSNode::last_node > MCTSNODE_POOL * 0.5) {
            mcts.reset();
        // }
        /*
        else 
        if (TURN > 0) {
            bool found = false;
            int8_t found_moves = -1;
            int found_count = 0;
            for (int8_t a = 0; a < 4; a++) {
                for (int8_t b = 0; b < 4; b++) {
                    State prev_state = last_state;
                    int8_t moves; 

                    if (PLAYER_IDX == 0) {
                        prev_state.play(last_move, a, b);
                        moves = last_move | (a << 2) | (b << 4);
                    }
                    else
                    if (PLAYER_IDX == 1) {
                        prev_state.play(a, last_move, b);
                        moves = a | (last_move << 2) | (b << 4);
                    }
                    else {
                        prev_state.play(a, b, last_move);
                        moves = a | (b << 2) | (last_move << 4);
                    }

                    prev_state.apply_places();

                    if (last_state.hurdle_race.end) {
                        for (int i = 0; i < 3; i++) {
                            prev_state.hurdle_race_score[i] = current_state.hurdle_race_score[i];
                        }
                    }

                    if (last_state.archery.end) {
                        for (int i = 0; i < 3; i++) {
                            prev_state.archery_score[i] = current_state.archery_score[i];
                        }
                    }

                    if (last_state.roller_skating.end) {
                        for (int i = 0; i < 3; i++) {
                            prev_state.roller_skating_score[i] = current_state.roller_skating_score[i];
                        }
                    }

                    if (last_state.diving.end) {
                        for (int i = 0; i < 3; i++) {
                            prev_state.diving_score[i] = current_state.diving_score[i];
                        }
                    }

                    if (prev_state == current_state) {
                        found = true;
                        found_moves = moves;
                        found_count++;
                    }
                }
            }
            
            std::cerr << "FOUND: " << found << ' ' << found_moves << '\n';
            std::cerr << "> " << move_list[found_moves & 3] << ' ' << move_list[(found_moves >> 2) & 3] << ' ' << move_list[(found_moves >> 4) & 3] << '\n';

            assert(found);
            // assert(found_count == 1);

            mcts.pass_move(found_moves);
        }
        // */

        // std::cerr << "pre mcts: " << MCTSNode::last_node << '\n';
        // mcts.debug();

#ifdef PSYLEAGUE
        mcts.run(current_state, 20);
        mcts.debug();
#else
        mcts.run(current_state, (TURN == 0 ? 600 : 45));
        mcts.debug();
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
        std::cerr << "pool: " << (float) MCTSNode::last_node / MCTSNODE_POOL << '\n';
        std::cerr << "last: " << MCTSNode::last_node << '\n';

        // int8_t greedy_moves[3];
        // current_state.greedy_moves(greedy_moves);
        // int move = greedy_moves[PLAYER_IDX];
        // std::cerr << "GREEDY: ";
        // for (int i = 0; i < 3; i++) {
        //     std::cerr << move_list[greedy_moves[i]] << ' ';
        // }
        // std::cerr << '\n';

        int move = mcts.best_move(PLAYER_IDX);

        last_state = current_state;
        last_move = move;

        std::cout << move_list[move] << std::endl;

        // if (TURN == 1) {
        //     return 0;
        // }
    }

}