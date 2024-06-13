#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "state.hpp"
#include "const.hpp"

int minmax(const State& state, int depth) {
    if (depth == 0) {
        return state.evaluate();
    }

    int best_score = -INF;
    for (int p1 = 0; p1 < 4; p1++) {
        
        int worst_score = +INF;
        for (int p2 = 0; p2 < 4; p2++) {
            for (int p3 = 0; p3 < 4; p3++) {
                State next_state = state;

                next_state.play(p1, p2, p3);

                int score = minmax(next_state, depth - 1);

                if (worst_score > score) {
                    worst_score = score;
                }
            }
        }

        if (best_score > worst_score) {
            best_score = worst_score;
        }
    }

    return best_score;
}

int best_move(const State& state, int depth) {

    int best_score = -INF;
    int best_move = -1;

    for (int p1 = 0; p1 < 4; p1++) {
        
        int worst_score = +INF;
        for (int p2 = 0; p2 < 4; p2++) {
            for (int p3 = 0; p3 < 4; p3++) {
                State next_state = state;

                next_state.play(p1, p2, p3);

                int score = minmax(next_state, depth - 1);

                if (worst_score > score) {
                    worst_score = score;
                }
            }
        }

        if (best_score > worst_score) {
            best_score = worst_score;
            best_move = p1;
        }
    }

    return best_move;
}

#endif // SEARCH_HPP