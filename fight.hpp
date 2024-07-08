#ifndef FIGHT_HPP
#define FIGHT_HPP

#include "dqn.hpp"

#include <string>

void fight(const std::string& modelA, const std::string& modelB) {
    DQNAgent agent1(modelA), agent2(modelB);

    for (int rep = 0; rep < GAMES; rep++) {
        State state;
        state.randomize();

        while (state.is_terminal()) {
            
        }
    }
}

#endif // FIGHT_HPP