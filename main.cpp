#include "state.hpp"
#include "search.hpp"

#include <iostream>
#include <string>
#include <vector>

int TURN;
int PLAYER_IDX;
int NB_GAMES;

int main() {

    std::cin >> PLAYER_IDX;
    std::cin.ignore();
    std::cin >> NB_GAMES;
    std::cin.ignore();

    for (TURN = 0; ; TURN++) {
        State current_state;
        
        for (int i = 0; i < 3; i++) {
            int final_score;
            std::cin >> final_score;

            if (i == PLAYER_IDX) {
                {
                    int gold, silver, bronze;
                    std::cin >> gold >> silver >> bronze;

                    current_state.hurdle_race_score = 3 * gold + silver;
                }
                {
                    int gold, silver, bronze;
                    std::cin >> gold >> silver >> bronze;

                    current_state.archery_score = 3 * gold + silver;
                }
                {
                    int gold, silver, bronze;
                    std::cin >> gold >> silver >> bronze;

                    current_state.roller_skating_score = 3 * gold + silver;
                }
                {
                    int gold, silver, bronze;
                    std::cin >> gold >> silver >> bronze;

                    current_state.diving_score = 3 * gold + silver;
                }
            }
            else {
                for (int j = 0; j < NB_GAMES; j++) {
                    int gold, silver, bronze;
                    std::cin >> gold >> silver >> bronze;
                }
            }
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
        
        int move = best_move(current_state, 1);

        std::vector<std::string> move_list = {
            "UP", "RIGHT", "DOWN", "LEFT"
        };

        std::cout << move_list[move] << std::endl;
    }

}