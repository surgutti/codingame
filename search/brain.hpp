#ifndef BRAIN_HPP
#define BRAIN_HPP

#include "../state.hpp"
#include "../const.hpp"
#include "../utils.hpp"
#include "../timer.hpp"

#include <cstdint>
#include <cmath>

struct BrainNode {
    
    static BrainNode pool[BRAIN_POOL];
    static uint32_t  last;

    BrainNode* sons;

    float    avg;
    // float    var;
    unsigned vis;

    uint8_t last_move;

    inline void init(const uint8_t _last_move) {
        sons = 0;
        avg = 0;
        // var = 0;
        vis = 0;
        last_move = _last_move;
    }

    inline void expand() {
        sons = (pool + last);

        for (uint8_t move = 0; move < 4; move++) {
            pool[last++].init(move);
        }

        // random shuffle sons
        std::swap(pool[last - 1], pool[last - 1 - fast_rand() % 4]);
        std::swap(pool[last - 2], pool[last - 2 - fast_rand() % 3]);
        std::swap(pool[last - 3], pool[last - 3 - fast_rand() % 2]);
    }

    inline BrainNode* select() const {
        for (int8_t move = 0; move < 4; move++) {
            if ((sons + move)->vis == 0) {
                return sons + move;
            }
        }

        float best_score = -INF;
        BrainNode* best_node = 0;

        float sqrt_log_node_vis = C * fastsqrtf(fastlogf(vis));
        for (int8_t move = 0; move < 4; move++) {
            BrainNode* node = sons + move;

            // float reward_variance = node->var / node->vis;
            // float variance_term = reward_variance + fastsqrtf(2 * fastlogf(vis) / node->vis);
            // float ucb_score = node->avg + sqrt_log_node_vis * rsqrt_fast(node->vis) * std::min(0.25f, variance_term);

            float ucb_score = node->avg + sqrt_log_node_vis * rsqrt_fast(node->vis);
            // float ucb_score = avg[player_idx][move] + sqrt_log_node_vis * rsqrt_fast(vis[player_idx][move]); // C * std::sqrt(log_node_vis / vis[player_idx][move]);

            if (best_score < ucb_score) {
                best_score = ucb_score;
                best_node = node;
            }
        }
        return best_node;
    }

    inline BrainNode* random_select() const {
        return (sons + (fast_rand() & 3));
    }

    inline int8_t best_move() const {
        float best_score = -INF;
        int8_t best_move = -1;

        for (int8_t move = 0; move < 4; move++) {
            BrainNode* node = (sons + move);

            if (best_score < node->vis) {
                best_score = node->vis;
                best_move = node->last_move;
            }
        }

        return best_move;
    }

    inline void apply(float reward) {
        // float delta = reward - avg;

        avg *= vis;
        avg += reward;
        vis += 1;
        avg /= vis;

        // var += delta * (reward - avg);
    }

    void debug() const {
        std::cerr << "VIS: " << vis << '\n';
        std::cerr << "AVG: " << avg << '\n';
        // std::cerr << "VAR: " << var << '\n';
        std::cerr << "last_move: " << int(last_move) << '\n';

        std::cerr << "sons:\n";

        // float sqrt_log_node_vis = fastsqrtf(fastlogf(vis));
        for (int8_t move = 0; move < 4; move++) {
            BrainNode* node = (sons + move);
            
            std::cerr << " mov > " << int(node->last_move) << '\n';
            std::cerr << " vis > " << node->vis << '\n';
            std::cerr << " avg > " << node->avg << '\n';
            // std::cerr << " var > " << node->var << '\n';

            // float reward_variance = node->var / node->vis;
            // float variance_term = reward_variance + fastsqrtf(2 * fastlogf(vis) / node->vis);
            // float ucb_score = node->avg + sqrt_log_node_vis * rsqrt_fast(node->vis) * std::min(0.25f, variance_term);

            // std::cerr << " ucb > " << ucb_score << '\n';
            
            std::cerr << '\n';
        }
    }
};

BrainNode BrainNode::pool[BRAIN_POOL];
uint32_t  BrainNode::last = 0;

struct Brain {
    BrainNode* roots[3];

    inline void reset() {
        BrainNode::last = 0;

        for (int i = 0; i < 3; i++) {
            roots[i] = &BrainNode::pool[BrainNode::last];
            roots[i]->init(0);

            BrainNode::last++;
        }
    }

    void optimize(BrainNode** heads, State &state, float* reward) {
        if (state.is_terminal()) {
            state.get_stats(reward[0], reward[1], reward[2]);
            
            for (int i = 0; i < 3; i++) {
                if (heads[i]) {
                    heads[i]->apply(reward[i]);
                }
            }
            return;
        }

        if (heads[0] == 0 && heads[1] == 0 && heads[2] == 0) {
            do {
                // uint8_t moves = fast_rand();
                // state.play(moves & 3, (moves >> 2) & 3, (moves >> 4) & 3);
                state.play(random_move(), random_move(), random_move());
            } while (!state.is_terminal());

            state.get_stats(reward[0], reward[1], reward[2]);
            return;
        }

        BrainNode* childs[3];
        int8_t moves[3];

        for (int i = 0; i < 3; i++) {
            if (heads[i] == 0 || heads[i]->vis == 0) {
                childs[i] = 0;
                moves[i] = random_move(); // fast_rand() & 3;
            }
            else {
                if (heads[i]->sons == 0) {
                    heads[i]->expand();
                }

                childs[i] = heads[i]->select();
                moves[i] = childs[i]->last_move;
            }
        }

        state.play(moves[0], moves[1], moves[2]);
    
        optimize(childs, state, reward);

        for (int i = 0; i < 3; i++) if (heads[i]) {
            heads[i]->apply(reward[i]);
        }
    }

    void random_walk(BrainNode** heads, State &state, float* reward) {
        if (state.is_terminal()) {
            state.get_stats(reward[0], reward[1], reward[2]);
            
            for (int i = 0; i < 3; i++) {
                if (heads[i]) {
                    heads[i]->apply(reward[i]);
                }
            }
            return;
        }

        if (heads[0] == 0 && heads[1] == 0 && heads[2] == 0) {
            do {
                // uint8_t moves = fast_rand();
                // state.play(moves & 3, (moves >> 2) & 3, (moves >> 4) & 3);
                state.play(random_move(), random_move(), random_move());
            } while (!state.is_terminal());

            state.get_stats(reward[0], reward[1], reward[2]);
            return;
        }

        BrainNode* childs[3];
        int8_t moves[3];

        for (int i = 0; i < 3; i++) {
            if (heads[i] == 0 || heads[i]->vis == 0) {
                childs[i] = 0;
                moves[i] = random_move(); // fast_rand() & 3;
            }
            else {
                if (heads[i]->sons == 0) {
                    heads[i]->expand();
                }

                childs[i] = heads[i]->random_select();
                moves[i] = childs[i]->last_move;
            }
        }

        state.play(moves[0], moves[1], moves[2]);
    
        random_walk(childs, state, reward);

        for (int i = 0; i < 3; i++) if (heads[i]) {
            heads[i]->apply(reward[i]);
        }
    }

    int best_move(int player_idx) const {
        return roots[player_idx]->best_move();
    }

    void debug() const {
        for (int i = 0; i < 3; i++) {
            roots[i]->debug();
        }
    }

    void run(const State& root_state, int timeout) {
        reset();

        do {
            State state = root_state;
            BrainNode* heads[3] = {roots[0], roots[1], roots[2]};
            float reward[3];

            random_walk(heads, state, reward);
        } while (timer.get_elapsed() < timeout * 0.15 &&
                 BrainNode::last + 40 < BRAIN_POOL);

        // debug();

        do {
            State state = root_state;
            BrainNode* heads[3] = {roots[0], roots[1], roots[2]};
            float reward[3];

            // maybe first 20% of time use on random rollouts from the begginning and then the rest
            optimize(heads, state, reward);
        } while (timer.get_elapsed() < timeout &&
                 BrainNode::last + 40 < BRAIN_POOL);
        
    }
};

#endif // BRAIN_HPP