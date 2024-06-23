#ifndef MCTS_HPP
#define MCTS_HPP

#include "../const.hpp"
#include "../state.hpp"
#include "../random.hpp"
#include "../timer.hpp"

#include <cstdint>
#include <cmath>
#include <iostream>

struct MCTSNode {
    static MCTSNode pool[MCTSNODE_POOL];
    static int last_node;

    State state;

    int first_son;

    uint8_t last_moves;

    float avg[3][4];
    unsigned vis[3][4];

    unsigned node_vis;

    uint8_t todo[3];

    inline void init(const State& parent_state, const uint8_t& _last_moves) {
        last_moves = _last_moves;
        first_son = -1;

        state = parent_state;
        state.play(_last_moves);

        for (int i = 0; i < 3; i++) {
            todo[i] = 0;
            for (int j = 0; j < 4; j++) {
                avg[i][j] = 0;
                vis[i][j] = 0;
            }
        }
    }

    inline void expand() {
        first_son = last_node;
        for (uint8_t moves = 0; moves < 64; moves++) {
            pool[last_node++].node_vis = 0;
        }
    }

    int best_move_per_player(int player_idx) const {
        float best_score = -INF;
        int best_move = -1;

        for (int move = 0; move < 4; move++) {
            float node_score = vis[player_idx][move];

            if (best_score < node_score) {
                best_score = node_score;
                best_move = move;
            }
        }

        return best_move;
    }

    MCTSNode* get_node(int8_t moves) {
        return &pool[first_son + moves];
    }

    inline int8_t select_per_player(int player_idx) {
        if (todo[player_idx] < 4) {
            return todo[player_idx]++;
        }

        float best_score = -INF;
        int8_t best_move = -1;
        
        const float sqrt_log_node_vis = fastsqrtf(fastlogf(node_vis));
        for (int8_t move = 0; move < 4; move++) {
            float ucb_score = avg[player_idx][move] + sqrt_log_node_vis * rsqrt_fast(vis[player_idx][move]);

            if (best_score < ucb_score) {
                best_score = ucb_score;
                best_move = move;
            }
        }

        return best_move;
    }

    inline MCTSNode* select() {
        const uint8_t moves = (select_per_player(0) << 0) |
                              (select_per_player(1) << 2) |
                              (select_per_player(2) << 4);

        MCTSNode* node = &pool[first_son + moves];

        if (node->node_vis == 0) {
            node->init(state, moves);
        }

        return node;
    }

    inline void apply_per_player(int player_idx, uint8_t move, float reward) {
        avg[player_idx][move] *= vis[player_idx][move];
        avg[player_idx][move] += reward;
        vis[player_idx][move] += 1;
        avg[player_idx][move] /= vis[player_idx][move];
    }

    inline void apply(uint8_t moves, const float* rewards) {
        uint8_t m0 = (moves >> 0) & 3;
        uint8_t m1 = (moves >> 2) & 3;
        uint8_t m2 = (moves >> 4) & 3;

        apply_per_player(0, m0, rewards[0]);
        apply_per_player(1, m1, rewards[1]);
        apply_per_player(2, m2, rewards[2]);

        node_vis += 1;
    }

    void debug() const {
        std::cerr << "NODE VIS: " << node_vis << '\n';
        for (int i = 0; i < 3; i++) {
            std::cerr << "PLAYER: " << i << '\n';
            for (int move = 0; move < 4; move++) {
                std::cerr << avg[i][move] << '/' << vis[i][move] << '\n'; // << "(" << var[i][move] << ") ";
            }
            std::cerr << '\n';
        }
    }
};

MCTSNode MCTSNode::pool[MCTSNODE_POOL];
int      MCTSNode::last_node = 0;

struct MCTS {
    MCTSNode* root;

    void mcts(MCTSNode* node, float* rewards) {
        
        if (node->state.is_terminal()) {
            State state = node->state;
            state.get_stats(rewards);
            node->node_vis++;
            return;
        }

        if (node->node_vis == 0) {
            State state = node->state;

            state.rollout();
            // do {
            //     // maybe only once play greedy move? then just random (for sake of optimization)
            //     // if (state.still_playing()) {
            //     //     state.play_greedy();
            //     // }
            //     // else {
            //         // uint8_t moves = fast_rand();
            //         // state.play(moves & 3, (moves >> 2) & 3, (moves >> 4) & 3);
                    
            //         state.play(random_move(), random_move(), random_move()); 
            //     // }
            // } while (!state.is_rollout_terminal());

            state.get_stats(rewards);
            node->node_vis++;
            return;
        }

        if (node->first_son == -1) {
            node->expand();
        }

        MCTSNode* child = node->select();

        mcts(child, rewards);

        node->apply(child->last_moves, rewards);
    }

    inline void reset(const State &starting_state) {
        MCTSNode::last_node = 0;
        root = &MCTSNode::pool[MCTSNode::last_node];
        root->node_vis = 0;
        root->init(state, 0);

        MCTSNode::last_node++;
    }

    int best_move(int player_idx) const {
        return root->best_move_per_player(player_idx);
    }

    void pass_move(int8_t moves) {
        root = root->get_node(moves);
    }

    void debug() const {
        root->debug();
    }

    void run(const State& root_state, int timeout) {
        reset(root_state);

        float rewards[3];
        do {
            mcts(root, rewards);
        } while (timer.get_elapsed() < timeout &&
                 MCTSNode::last_node + 64 < MCTSNODE_POOL);
    }
};

#endif // MCTS_HPP