# Arena Tuning Workflow

This repo includes a local arena runner for benchmarking bot variants against each other.

## Requirements
- Python 3.9+
- `g++` with C++20 support
- Java 17+ (required by `referee.jar`)

If Java is too old, `tools/arena.py` will stop with a clear error message.
You can override Java executable with `JAVA_BIN=/path/to/java17`.

## Quick Start
1. Edit variant list in `configs/arena_variants.json`.
2. Build variants:
   - `make arena-build`
3. Run tournament:
   - `make arena-run`
4. Read results:
   - `arena/results/latest.md`
   - `arena/results/latest.json`

Or run build+run in one step:
- `make arena`

Closed-loop optimization:
- `make auto-improve`
- or `scripts/bot.sh auto-improve 6 10 3 12 4`
  - args: generations, population, elite, games, workers
  - output: `arena/auto/latest_summary.json`, `arena/auto/best_config.json`
- Promote best artifacts:
  - `make promote-best`
  - outputs: `build/brain_best`, `build/bot_submission_best.cpp`

## Useful Commands
- `python3 tools/arena.py --help`
- `python3 tools/arena.py --config configs/arena_variants.json --games 50 --workers 4`
- `python3 tools/arena.py --run-only --games 100 --workers 6`
- `python3 tools/auto_improve.py --generations 6 --population 10 --elite 3 --games 12 --workers 4`

## Variant Parameters
Use `defines` in `configs/arena_variants.json` to override compile-time macros:
- `BOT_TIMEOUT_SCALE_PCT`
- `BOT_UCB_C`
- `BOT_RANDOM_PHASE_RATIO`
- `BOT_TIMER_CHECK_MASK`
- `BOT_GREEDY_W0`, `BOT_GREEDY_W1`, `BOT_GREEDY_W2`, `BOT_GREEDY_W3`

The defaults are in `src/bot/core/const.hpp`.

`configs/arena_variants.json` uses `-DLOCAL` by default for portable local builds.
If your machine/toolchain supports the same target flags as CodinGame, you can remove
`-DLOCAL` from `cxxflags` for a closer production match.

## Output Metrics
Leaderboard includes:
- `FIT`: optimizer fitness (`points_per_game - 1.5 * error_rate`)
- `P/G`: points per game (3/1/0 for 1st/2nd/3rd)
- `WR`: first-place rate
- `TOP2`: top-2 rate
- `AVG_RANK`: lower is better
- `ERR%`: negative-score rate

`latest.json` also includes pairwise win/tie counts and top variant defines.
