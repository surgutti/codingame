# Post-mortem Notes Applied

Date: 2026-03-05

## Sources
- https://forum.codingame.com/t/summer-challenge-2024-feedback-and-strategies/204165
- https://www.yannmoisan.com/summer-challenge-2024.html
- https://gist.github.com/descampsk/1b062565941ed963b94f44ce53af1916#summer-challenge-2024-postmortem

## Ideas integrated into code

1. Speed/timing as first-class tuning axis
- Added compile-time timeout scaling macro (`BOT_TIMEOUT_SCALE_PCT`) to generate 100/125/150/200% variants quickly.
- Added closed-loop auto tuning (`tools/auto_improve.py`) to continuously test timing/heuristic combinations.

2. No-regeneration rollout mode (post-mortem style)
- Added `BOT_REGENERATE_ENDED_GAMES` toggle.
- When disabled, rollout does not spawn synthetic minigames after a game ends.

3. Inverse-score prioritization mode
- Added `BOT_WEIGHT_MODE`.
- Mode 1 changes greedy rollout vote weighting to prioritize minigames where own score is lower.

4. Faster/stabler local experimentation
- Arena now auto-selects Java 17 (`/usr/lib/jvm/java-17-openjdk/bin/java`) and validates runtime version before matches.
- Added reliable single-worker run path for constrained environments (`workers=1`).

## Related tunables exposed
- `BOT_UCB_C`
- `BOT_RANDOM_PHASE_RATIO`
- `BOT_TIMER_CHECK_MASK`
- `BOT_ROLLOUT_GREEDY_RANDOM_PCT`
- `BOT_GREEDY_W0..BOT_GREEDY_W3`
