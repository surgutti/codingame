# Architecture and File Flow

## Source of truth
- Edit files under `src/bot/**`.
- Do not edit `bot.cpp` directly.

Dependency flow:
1. `src/bot/main.cpp`
2. `src/bot/search/brain.hpp`
3. `src/bot/state/state.hpp`
4. `src/bot/minigames/*.hpp`
5. `src/bot/core/*.hpp`

## Submission flow
1. Bundle command expands local `#include "..."` recursively:
   - `python3 tools/bundle_submission.py brain.cpp --output build/bot_submission.cpp`
2. Optional overwrite of competition file:
   - `python3 tools/bundle_submission.py brain.cpp --output bot.cpp`

`brain.cpp` is now a thin compatibility entrypoint that includes `src/bot/main.cpp`.

## Legacy compatibility
Root headers (`state.hpp`, `brain.hpp`, `const.hpp`, etc.) and `minigames/*.hpp` are wrappers that include the canonical files in `src/`.
This keeps old scripts and include paths working while allowing cleaner organization.
