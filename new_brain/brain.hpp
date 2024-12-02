#ifndef BRAIN_HPP
#define BRAIN_HPP

#include "state.hpp"
#include "const.hpp"
#include "utils.hpp"
#include "timer.hpp"

#include <cstdint>
#include <cmath>

struct 

struct BrainNode {
	
	BrainNode* sons[4];
	
	float sum[4];
	unsigned vis[4];



};

struct BrainNode {
    
    static BrainNode pool[BRAIN_POOL];
    static uint32_t  last;

    BrainNode* sons;

    float    avg;
    unsigned vis;

    uint8_t last_move;

    uint8_t todo;

    inline void init(const uint8_t _last_move) {
        sons = 0;
        avg = 0;
        vis = 0;
        last_move = _last_move;
        todo = 0;
    }

    inline void expand() {
        sons = (pool + last);

        for (uint8_t move = 0; move < 4; move++) {
            pool[last++].init(move);
        }
        
        // std::swap(pool[last - 1], pool[last - 1 - (fast_rand() & 3)]);
        // std::swap(pool[last - 2], pool[last - 2 - (fast_rand() % 3)]);
        // std::swap(pool[last - 3], pool[last - 3 - (fast_rand() & 1)]);
    }

    inline BrainNode* select() {
        if (todo < 4) {
            return sons + (todo++);
        }

        const float sqrt_log_node_vis = C * fastsqrtf(fastlogf(vis));

        float best_score = -INF;
        BrainNode* best_node = 0;

        for (int8_t move = 0; move < 4; move++) {
            BrainNode* node = (sons + move);

            const float ucb_score = node->avg + sqrt_log_node_vis * rsqrt_fast(node->vis);

            if (best_score < ucb_score) {
                best_score = ucb_score;
                best_node = node;
            }
        }

        return best_node;
    }

    inline BrainNode* random_select() const {
        return (sons + random_move());
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
        avg *= vis;
        avg += reward;
        vis += 1;
        avg /= vis;
    }

    void debug() const {
        std::cerr << "VIS: " << vis << '\n';
        std::cerr << "AVG: " << avg << '\n';
        std::cerr << "last_move: " << int(last_move) << '\n';

        std::cerr << "sons:\n";

        float sqrt_log_node_vis = fastsqrtf(fastlogf(vis));
        for (int8_t move = 0; move < 4; move++) {
            BrainNode* node = (sons + move);
            
            std::cerr << " mov > " << int(node->last_move) << '\n';
            std::cerr << " vis > " << node->vis << '\n';
            std::cerr << " avg > " << node->avg << '\n';

            float ucb_score = node->avg + sqrt_log_node_vis * rsqrt_fast(node->vis);

            std::cerr << " ucb > " << ucb_score << '\n';
            
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

    inline void optimize(BrainNode** heads, State &state, float* reward) {

        if (state.is_terminal()) {
            state.get_stats(reward);
            
            for (int i = 0; i < 3; i++) {
                if (heads[i]) {
                    heads[i]->apply(reward[i]);
                }
            }
            return;
        }

        if (heads[0] == 0 && heads[1] == 0 && heads[2] == 0) {
            while (!state.is_rollout_terminal()) {
                state.play_greedy();
            }

            state.get_stats(reward);
            return;
        }

        BrainNode* childs[3];
        int8_t moves[3];

        for (int i = 0; i < 3; i++) {
            if (heads[i] == 0 || heads[i]->vis == 0) {
                childs[i] = 0;
                moves[i] = state.greedy_move(i);
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

    inline void random_walk(BrainNode** heads, State &state, float* reward) {
        if (state.is_terminal()) {
            state.get_stats(reward);
            
            for (int i = 0; i < 3; i++) {
                if (heads[i]) {
                    heads[i]->apply(reward[i]);
                }
            }
            return;
        }

        if (heads[0] == 0 && heads[1] == 0 && heads[2] == 0) {
            while (!state.is_rollout_terminal()) {
                state.play_random();
            }

            state.get_stats(reward);
            return;
        }

        BrainNode* childs[3];
        int8_t moves[3];

        for (int i = 0; i < 3; i++) {
            if (heads[i] == 0 || heads[i]->vis == 0) {
                childs[i] = 0;
                moves[i] = state.greedy_move(i);
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

    inline int best_move(int player_idx) const {
        return roots[player_idx]->best_move();
    }

    void debug() const {
        for (int i = 0; i < 3; i++) {
            roots[i]->debug();
        }
    }

    void run(const State& root_state, int timeout) {
        reset();

        State state;
        float reward[3];
        BrainNode* heads[3] = {roots[0], roots[1], roots[2]};
        
        do {
            state = root_state;
            random_walk(heads, state, reward);
        } while (timer.get_elapsed() < timeout * 0.12 &&
                 BrainNode::last + 40 < BRAIN_POOL);
        
        do {
            state = root_state;
            optimize(heads, state, reward);
        } while (timer.get_elapsed() < timeout &&
                 BrainNode::last + 40 < BRAIN_POOL);
    }
};

#endif // BRAIN_HPP
