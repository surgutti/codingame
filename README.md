# Olympbits Bot (Summer Challenge 2024)

## Source layout
- `src/bot/main.cpp`: main loop and CodinGame I/O.
- `src/bot/search/brain.hpp`: search tree and optimization loop.
- `src/bot/state/state.hpp`: combined game state + reward calculation.
- `src/bot/minigames/*.hpp`: hurdles/archery/skating/divings logic.
- `src/bot/core/*.hpp`: constants, RNG, utility math, timer.

This means old scripts still work, but edits should go to `src/bot/**`.

## Workflow
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
