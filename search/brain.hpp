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
    float    var;
    unsigned vis;

    uint8_t last_move;

    inline void init(const uint8_t _last_move) {
        avg = 0;
        var = 0;
        vis = 0;
        last_move = _last_move;
    }

    inline void expand() {
        sons = (pool + last);
        for (uint8_t move = 0; move < 4; move++) {
            pool[last++].init(move);
        }
    }

    inline BrainNode* select() const {
        for (int8_t move = 0; move < 4; move++) {
            if ((sons + move)->vis == 0) {
                return sons + move;
            }
        }

        float best_score = -INF;
        BrainNode* best_node = 0;

        float sqrt_log_node_vis = fastsqrtf(fastlogf(vis));
        for (int8_t move = 0; move < 4; move++) {
            BrainNode* node = sons + move;

            float reward_variance = node->var / node->vis;
            float variance_term = reward_variance + fastsqrtf(2 * fastlogf(vis) / node->vis);
            float ucb_score = node->avg + sqrt_log_node_vis * rsqrt_fast(node->vis) * std::min(0.25f, variance_term);

            // float ucb_score = avg[player_idx][move] + sqrt_log_node_vis * rsqrt_fast(vis[player_idx][move]); // C * std::sqrt(log_node_vis / vis[player_idx][move]);

            if (best_score < ucb_score) {
                best_score = ucb_score;
                best_node = node;
            }
        }
        return best_node;
    }

    inline void apply(float reward) {
        float delta = reward - avg;

        avg *= vis;
        avg += reward;
        vis += 1;
        avg /= vis;

        var += delta * (reward - avg);
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

        BrainNode::last++;
    }

    void optimize(BrainNode** heads, State &state, float* reward) {
        if (state.is_terminal()) {
            state.get_stats(reward[0], reward[1], reward[2]);
            return;
        }

        for (int i = 0; i < 3; i++) {
            if (heads[i]->vis == 0) {
                heads[i]->expand();
            }

            heads[i] = heads[i]->select();
        }

         state.play(heads[0]->last_move,
                    heads[1]->last_move,
                    heads[2]->last_move);
        
        optimize(heads, state, reward);

        heads[0]->apply(reward[0]);
        heads[1]->apply(reward[1]);
        heads[2]->apply(reward[2]);
    }

    void run(const State& root_state, int timeout) {
        reset();

        do {
            BrainNode* heads[3] = {roots[0], roots[1], roots[2]};


        } while (timer.get_elapsed() < timeout &&
                 BrainNode::last + 3 < BRAIN_POOL);
    }
};

#endif // BRAIN_HPP