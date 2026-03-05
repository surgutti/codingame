# Olympbits Bot (Summer Challenge 2024)

This repository now has a clear source-of-truth layout for the 20th-place competition bot while keeping old paths compatible.

## Canonical source layout
- `src/bot/main.cpp`: main loop and CodinGame I/O.
- `src/bot/search/brain.hpp`: search tree and optimization loop.
- `src/bot/state/state.hpp`: combined game state + reward calculation.
- `src/bot/minigames/*.hpp`: hurdles/archery/skating/divings logic.
- `src/bot/core/*.hpp`: constants, RNG, utility math, timer.

## Compatibility layer (legacy paths)
- `brain.cpp`, `brain.hpp`, `state.hpp`, `const.hpp`, `random.hpp`, `utils.hpp`, `timer.hpp`, `minigames/*.hpp`
  are lightweight wrappers that include files from `src/`.

This means old scripts still work, but edits should go to `src/bot/**`.

## Submission workflow
- Build local bot:
  - `make build-bot` (builds with `-DLOCAL`)
- Run tests:
  - `make test`
- Run performance benchmark:
  - `make bench` (runs search throughput benchmark; args via `./build/perf_bench <runs> <timeout_ms>`)
- Syntax-check:
  - `make check`
- Bundle single-file submission:
  - `make bundle` (output: `build/bot_submission.cpp`)
- Refresh `bot.cpp` from current source:
  - `make refresh-bot`
- Arena benchmark (build variants + run round-robin):
  - `make arena-build` (compile variants from `configs/arena_variants.json`)
  - `make arena-run` (run tournament from existing manifest)
  - `make arena` (build + run)
  - `make auto-improve` (closed-loop evolutionary self-play tuning)
  - `make promote-best` (compile + bundle best found parameters)
  - reports: `arena/results/latest.json` and `arena/results/latest.md`
  - note: local referee requires Java 17+ (`referee.jar` is classfile 61)
  - full guide: `docs/arena.md`
  - applied post-mortem mapping: `docs/postmortem_notes.md`

You can also use the helper script:
- `scripts/bot.sh build|test|check|bench|bundle|refresh-bot|arena*|auto-improve|promote-best|league-*`

## Psyleague workflow
Based on `https://github.com/FakePsyho/psyleague` command model:
- Add bot from local source:
  - `scripts/bot.sh league-add my_bot brain`
- Run league:
  - `scripts/bot.sh league-run`
- Show leaderboard:
  - `scripts/bot.sh league-show`
- Remove bot:
  - `scripts/bot.sh league-remove my_bot`

`psyleague.cfg` remains the configuration source for compile/run behavior.

## Legacy/experimental folders
- `bots/`: archived local league bots + compiled binaries.
- `old_bots/`: older bot generations.
- `gen_bots/`: generated coefficient variants.
- `mcts_state_keeping/`, `mcts/mcts.hpp`, `new_brain/brain.hpp`: unfinished branches (see `docs/non_working_code.md`).

## Competition/runtime assumptions
- Compiler/runtime target: `g++ 11.2.0`, C++20.
- Memory limit: 768 MB.
- Single-core timing behavior matters in practice.
- `#pragma GCC target(...)` assumes x86_64 with AVX2-class support (matching the provided Haswell-like environment).
