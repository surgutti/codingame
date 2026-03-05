#ifndef BRAIN_HPP
#define BRAIN_HPP

#include "../state/state.hpp"
#include "../core/const.hpp"
#include "../core/utils.hpp"
#include "../core/timer.hpp"

#include <cstdint>
#include <cmath>

struct BrainNode {
    
    static BrainNode pool[BRAIN_POOL];
    static uint32_t  last;
    static constexpr uint32_t kNoSons = 0xFFFFFFFFu;

    uint32_t sons_index;
    uint32_t vis;
    float avg;

    uint8_t last_move;
    uint8_t todo;

    inline void init(uint8_t _last_move) {
        sons_index = kNoSons;
        avg = 0;
        vis = 0;
        last_move = _last_move;
        todo = 0;
    }

    inline BrainNode* sons_ptr() {
        return pool + sons_index;
    }

    inline const BrainNode* sons_ptr() const {
        return pool + sons_index;
    }

    inline bool is_expanded() const {
        return sons_index != kNoSons;
    }

    inline void expand() {
        sons_index = last;

        for (uint8_t move = 0; move < MOVE_COUNT; move++) {
            pool[last++].init(move);
        }
        
        // std::swap(pool[last - 1], pool[last - 1 - (fast_rand() & 3)]);
        // std::swap(pool[last - 2], pool[last - 2 - (fast_rand() % 3)]);
        // std::swap(pool[last - 3], pool[last - 3 - (fast_rand() & 1)]);
    }

    inline BrainNode* select() {
        BrainNode* sons = sons_ptr();
        if (todo < MOVE_COUNT) {
            return sons + (todo++);
        }

        // UCB with fast approximations tuned for rollout throughput.
        const float sqrt_log_node_vis = C * fastsqrtf(fastlogf(vis));

        float best_score = -INF;
        BrainNode* best_node = 0;

        for (int8_t move = 0; move < MOVE_COUNT; move++) {
            BrainNode* node = sons + move;

            const float ucb_score = node->avg + sqrt_log_node_vis * rsqrt_fast(node->vis);

            if (best_score < ucb_score) {
                best_score = ucb_score;
                best_node = node;
            }
        }

        return best_node;
    }

    inline BrainNode* random_select() {
        return sons_ptr() + random_move();
    }

    inline int8_t best_move() const {
        const BrainNode* sons = sons_ptr();
        int best_vis = -1;
        int8_t best_move = -1;

        for (int8_t move = 0; move < MOVE_COUNT; move++) {
            const BrainNode* node = sons + move;

            if (best_vis < static_cast<int>(node->vis)) {
                best_vis = static_cast<int>(node->vis);
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
        const BrainNode* sons = sons_ptr();
        for (int8_t move = 0; move < MOVE_COUNT; move++) {
            const BrainNode* node = sons + move;
            
            std::cerr << " mov > " << int(node->last_move) << '\n';
            std::cerr << " vis > " << node->vis << '\n';
            std::cerr << " avg > " << node->avg << '\n';

            float ucb_score = node->avg + sqrt_log_node_vis * rsqrt_fast(node->vis);

            std::cerr << " ucb > " << ucb_score << '\n';
            
            std::cerr << '\n';
        }
    }
};

static_assert(sizeof(BrainNode) <= 16, "BrainNode should remain compact");

BrainNode BrainNode::pool[BRAIN_POOL];
uint32_t  BrainNode::last = 0;

struct Brain {
    BrainNode* roots[PLAYER_COUNT];

    static inline bool no_heads(const BrainNode* const* heads) {
        return heads[0] == 0 && heads[1] == 0 && heads[2] == 0;
    }

    static inline void apply_rewards(BrainNode** heads, const float* reward) {
        if (heads[0]) heads[0]->apply(reward[0]);
        if (heads[1]) heads[1]->apply(reward[1]);
        if (heads[2]) heads[2]->apply(reward[2]);
    }

    inline void reset() {
        BrainNode::last = 0;

        for (int i = 0; i < PLAYER_COUNT; i++) {
            roots[i] = &BrainNode::pool[BrainNode::last];
            roots[i]->init(0);

            BrainNode::last++;
        }
    }

    inline void optimize(BrainNode** heads, State &state, float* reward) {

        if (state.is_terminal()) {
            state.get_stats(reward);
            apply_rewards(heads, reward);
            return;
        }

        if (no_heads(heads)) {
            // No tree nodes left for this branch, finish with greedy rollout.
            while (!state.is_rollout_terminal()) {
                state.play_greedy();
            }

            state.get_stats(reward);
            return;
        }

        BrainNode* childs[PLAYER_COUNT];
        int8_t moves[PLAYER_COUNT];

        for (int i = 0; i < PLAYER_COUNT; i++) {
            if (heads[i] == 0 || heads[i]->vis == 0) {
                childs[i] = 0;
                moves[i] = state.greedy_move(i);
            }
            else {
                if (!heads[i]->is_expanded()) {
                    heads[i]->expand();
                }

                childs[i] = heads[i]->select();
                moves[i] = childs[i]->last_move;
            }
        }

        state.play(moves[0], moves[1], moves[2]);
    
        optimize(childs, state, reward);

        apply_rewards(heads, reward);
    }

    inline void random_walk(BrainNode** heads, State &state, float* reward) {
        if (state.is_terminal()) {
            state.get_stats(reward);
            apply_rewards(heads, reward);
            return;
        }

        if (no_heads(heads)) {
            // Fast random rollout phase used before optimize().
            while (!state.is_rollout_terminal()) {
                state.play_random();
            }

            state.get_stats(reward);
            return;
        }

        BrainNode* childs[PLAYER_COUNT];
        int8_t moves[PLAYER_COUNT];

        for (int i = 0; i < PLAYER_COUNT; i++) {
            if (heads[i] == 0 || heads[i]->vis == 0) {
                childs[i] = 0;
                moves[i] = state.greedy_move(i);
            }
            else {
                if (!heads[i]->is_expanded()) {
                    heads[i]->expand();
                }

                childs[i] = heads[i]->random_select();
                moves[i] = childs[i]->last_move;
            }
        }

        state.play(moves[0], moves[1], moves[2]);
    
        random_walk(childs, state, reward);

        apply_rewards(heads, reward);
    }

    inline int best_move(int player_idx) const {
        return roots[player_idx]->best_move();
    }

    void debug() const {
        for (int i = 0; i < PLAYER_COUNT; i++) {
            roots[i]->debug();
        }
    }

    void run(const State& root_state, int timeout) {
        reset();

        State state;
        float reward[PLAYER_COUNT];
        BrainNode* heads[PLAYER_COUNT] = {roots[0], roots[1], roots[2]};
        int loop_counter = 0;
        const int random_phase_timeout = static_cast<int>(timeout * RANDOM_PHASE_RATIO);

        while (BrainNode::last + BRAIN_POOL_GUARD < BRAIN_POOL) {
            state = root_state;
            random_walk(heads, state, reward);

            if ((++loop_counter & TIMER_CHECK_MASK) == 0 &&
                timer.get_elapsed() >= random_phase_timeout) {
                break;
            }
        }

        loop_counter = 0;
        while (BrainNode::last + BRAIN_POOL_GUARD < BRAIN_POOL) {
            state = root_state;
            optimize(heads, state, reward);

            if ((++loop_counter & TIMER_CHECK_MASK) == 0 &&
                timer.get_elapsed() >= timeout) {
                break;
            }
        }
    }
};

#endif // BRAIN_HPP
