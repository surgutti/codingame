#ifndef CONST_HPP
#define CONST_HPP

#ifndef BOT_UCB_C
    #define BOT_UCB_C 0.4f
#endif

#ifndef BOT_RANDOM_PHASE_RATIO
    #define BOT_RANDOM_PHASE_RATIO 0.12f
#endif

#ifndef BOT_TIMEOUT_SCALE_PCT
    #define BOT_TIMEOUT_SCALE_PCT 100
#endif

#ifndef BOT_TIMEOUT_TURN0_MS
    #define BOT_TIMEOUT_TURN0_MS 600
#endif

#ifndef BOT_TIMEOUT_TURN_MS
    #define BOT_TIMEOUT_TURN_MS 45
#endif

#ifndef BOT_TIMEOUT_PSYLEAGUE_MS
    #define BOT_TIMEOUT_PSYLEAGUE_MS 20
#endif

#ifndef BOT_TIMER_CHECK_MASK
    #define BOT_TIMER_CHECK_MASK 3
#endif

#ifndef BOT_WEIGHT_MODE
    #define BOT_WEIGHT_MODE 0
#endif

#ifndef BOT_REGENERATE_ENDED_GAMES
    #define BOT_REGENERATE_ENDED_GAMES 1
#endif

#ifndef BOT_ROLLOUT_GREEDY_RANDOM_PCT
    #define BOT_ROLLOUT_GREEDY_RANDOM_PCT 0
#endif

#ifndef BOT_GREEDY_W0
    #define BOT_GREEDY_W0 0.031946663887537924f
#endif

#ifndef BOT_GREEDY_W1
    #define BOT_GREEDY_W1 0.018851756640959727f
#endif

#ifndef BOT_GREEDY_W2
    #define BOT_GREEDY_W2 0.02367701892123486f
#endif

#ifndef BOT_GREEDY_W3
    #define BOT_GREEDY_W3 0.02552456055026749f
#endif

constexpr int PLAYER_COUNT = 3;
constexpr int MOVE_COUNT = 4;
constexpr int GAME_COUNT = 4;
constexpr int MAX_TURNS = 100;

constexpr int INF = 1'000'000;

constexpr int TRACK_LENGTH = 30;
constexpr int ARCHERY_LENGTH = 12 + 3; // 12 + random.nextInt(4);
constexpr int DIVING_LENGTH = 12 + 3; // 12 + random.nextInt(4);
constexpr int ARCHERY_GRID_RADIUS = 20;
constexpr int ARCHERY_GRID_SIZE = 2 * ARCHERY_GRID_RADIUS + 1;
constexpr int REG_FIELDS = 7;

constexpr int MCTSNODE_POOL = 6'000'000;
constexpr int BRAIN_POOL = 10'000'000;
constexpr int BRAIN_POOL_GUARD = 40;

constexpr float C = BOT_UCB_C;
constexpr float RANDOM_PHASE_RATIO = BOT_RANDOM_PHASE_RATIO;
constexpr int TIMEOUT_SCALE_PCT = BOT_TIMEOUT_SCALE_PCT;
constexpr int TIMEOUT_TURN0_MS = BOT_TIMEOUT_TURN0_MS;
constexpr int TIMEOUT_TURN_MS = BOT_TIMEOUT_TURN_MS;
constexpr int TIMEOUT_PSYLEAGUE_MS = BOT_TIMEOUT_PSYLEAGUE_MS;
constexpr int TIMER_CHECK_MASK = BOT_TIMER_CHECK_MASK;
constexpr int WEIGHT_MODE = BOT_WEIGHT_MODE;
constexpr int REGENERATE_ENDED_GAMES = BOT_REGENERATE_ENDED_GAMES;
constexpr int ROLLOUT_GREEDY_RANDOM_PCT = BOT_ROLLOUT_GREEDY_RANDOM_PCT;

inline int PLAYER_IDX;

constexpr float UP_P = 0.25f;
constexpr float LEFT_P = 0.25f;
constexpr float DOWN_P = 0.25f;
constexpr float RIGHT_P = 0.25f;

constexpr float MOVE_P[MOVE_COUNT] = {
    UP_P, LEFT_P, DOWN_P, RIGHT_P
};

constexpr float GREEDY_BASE_WEIGHTS[MOVE_COUNT] = {
    BOT_GREEDY_W0,
    BOT_GREEDY_W1,
    BOT_GREEDY_W2,
    BOT_GREEDY_W3
};

#endif // CONST_HPP
