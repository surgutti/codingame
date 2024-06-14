#ifndef MCTS_HPP
#define MCTS_HPP

#include "const.hpp"
#include "state.hpp"
#include "random.hpp"
#include "timer.hpp"

#include <cstdint>
#include <cmath>
#include <iostream>

struct MCTSNode {
    static MCTSNode pool[MCTSNODE_POOL];
    static int last_node;

    int first_son;

    uint8_t last_moves;

    float avg[3][4];
    unsigned vis[3][4];

    unsigned node_vis;

    void init(const uint8_t& _last_moves) {
        last_moves = _last_moves;
        first_son = -1;
        node_vis = 0;

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 4; j++) {
                avg[i][j] = 0;
                vis[i][j] = 0;
            }
        }
    }

    void expand() {
        first_son = last_node;
        for (uint8_t moves = 0; moves < 64; moves++) {
            pool[last_node++].init(moves);
        }
    }

    int best_move_per_player(int player_idx) const {
        float best_score = -INF;
        int best_move = -1;

        for (int move = 0; move < 4; move++) {
            float node_score = vis[player_idx][move]; // avg[player_idx][move];

            if (best_score < node_score) {
                best_score = node_score;
                best_move = move;
            }
        }

        return best_move;
    }

    int select_per_player(int player_idx) const {
        float best_score = -INF;
        int best_move = -1;
        
        for (int move = 0; move < 4; move++) {
            if (vis[player_idx][move] == 0) {
                return move;
            }

            float node_score = avg[player_idx][move] + C * std::sqrt(std::log(node_vis) / vis[player_idx][move]);
        
            if (best_score < node_score) {
                best_score = node_score;
                best_move = move;
            }
        }

        return best_move;
    }

    MCTSNode* select() const {
        return &pool[first_son + select_per_player(0) * 1
                               + select_per_player(1) * 4
                               + select_per_player(2) * 16];
    }

    void apply(uint8_t moves, float r1, float r2, float r3) {
        int m0 = (moves >> 0) & 3;
        int m1 = (moves >> 2) & 3;
        int m2 = (moves >> 4) & 3;

        avg[0][m0] *= vis[0][m0];
        avg[0][m0] += r1;
        vis[0][m0] += 1;
        avg[0][m0] /= vis[0][m0];

        avg[1][m1] *= vis[1][m1];
        avg[1][m1] += r2;
        vis[1][m1] += 1;
        avg[1][m1] /= vis[1][m1];

        avg[2][m2] *= vis[2][m2];
        avg[2][m2] += r3;
        vis[2][m2] += 1;
        avg[2][m2] /= vis[2][m2];

        node_vis += 1;
    }

    void debug() const {
        std::cerr << "NODE VIS: " << node_vis << '\n';
        for (int i = 0; i < 3; i++) {
            std::cerr << "PLAYER: " << i << '\n';
            for (int move = 0; move < 4; move++) {
                std::cerr << avg[i][move] << '/' << vis[i][move] << ' ';
            }
            std::cerr << '\n';
        }
    }
};

MCTSNode MCTSNode::pool[MCTSNODE_POOL];
int      MCTSNode::last_node = 0;

struct MCTS {
    MCTSNode* root;

    void rollout(State& state, float& r0, float& r1, float& r2) {
        do {
            state.play(fast_rand() & 3, fast_rand() & 3, fast_rand() & 3);
        } while (!state.is_terminal());

        state.get_stats(r0, r1, r2);
    }

    void mcts(MCTSNode* node, State& state, float& r0, float& r1, float& r2) {
        if (state.is_terminal()) {
            state.get_stats(r0, r1, r2);
            return;
        }

        if (node->node_vis == 0) {
            rollout(state, r0, r1, r2);
            node->node_vis++;
            return;
        }

        if (node->first_son == -1) {
            node->expand();
        }

        MCTSNode* child = node->select();

        int m0 = (child->last_moves >> 0) & 3;
        int m1 = (child->last_moves >> 2) & 3;
        int m2 = (child->last_moves >> 4) & 3;

        state.play(m0, m1, m2);

        mcts(child, state, r0, r1, r2);

        node->apply(child->last_moves, r0, r1, r2);
    }

    void reset() {
        MCTSNode::last_node = 0;
        MCTSNode::pool[MCTSNode::last_node].init(0);
        root = &MCTSNode::pool[MCTSNode::last_node];
        MCTSNode::last_node++;
    }

    int best_move(int player_idx) const {
        return root->best_move_per_player(player_idx);
    }

    void debug() {
        root->debug();
    }

    void run(const State& root_state, int timeout) {
        reset();

        // root_state.debug();
        // return;

        do {
            State state = root_state;
            float r0, r1, r2;
            mcts(root, state, r0, r1, r2);
        } while (timer.get_elapsed() < timeout &&
                 MCTSNode::last_node + 80 < MCTSNODE_POOL);
    }
};

#endif // MCTS_HPP