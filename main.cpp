#include "state.hpp"
#include "const.hpp"

State state;

int main() {
    state.init();

    for (; ; TURN++) {
        state.read();

        if (TURN == 0) {
            for (int i = 0; i < PODS_NB; i++) {
                state.pods[i].angle = atan2f(state.checkpoints[state.pods[i].cp_next].y - state.pods[i].y,
                                             state.checkpoints[state.pods[i].cp_next].x - state.pods[i].x);
            }
        }

        state.debug();

        
    }


    return 0;
}