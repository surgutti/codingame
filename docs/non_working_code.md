# Non-working Code Report

Audit date: 2026-03-05

## Working compile targets (syntax check)
- `bot.cpp`
- `brain.cpp`
- `cg.cpp`
- `test_fun.cpp`

Checked with:
- `g++ -std=c++20 -O2 -fsyntax-only <file>`

## Failing targets

### `mcts_state_keeping/mcts.cpp` + `mcts_state_keeping/mcts.hpp`
Current code does not match the active `State` API and minigame type names.

Main failures:
- Calls missing API: `State::play(uint8_t)`, `State::rollout()`.
- Uses undefined symbols from older code: `RollerSkating`, `Diving`, `HurdleRace`.
- Uses removed state fields: `hurdle_race_score`, `roller_skating_score`, `diving_score`.
- `MCTS::reset()` is called without required argument.

### `mcts/mcts.hpp`
Header is incomplete/non-buildable.

Main failures:
- Undefined macro: `INLINE`.
- Syntax error in SIMD block (`auto v = _mm_load_ps(vis):`).
- Missing members referenced in `init()` (`nodes`, `root`, `nodes_count`).
- Unknown intrinsic name typo (`_m_fmadd_ps`).

### `new_brain/brain.hpp`
Header is malformed and redefines `BrainNode`.

Main failures:
- Stray `struct` declaration.
- Duplicate `BrainNode` definitions.
- Static member definitions do not match the first malformed struct.
- Downstream member accesses fail because the malformed first definition is used.

## Warning-level issue in active code

### `state.hpp` and `bot.cpp`
In `State::greedy_move`, diving weight has a split statement:

```
(skating_score[player_idx] + 1); + 1.0f / (1 + pop_count(moves));
```

The `+ 1.0f / (...)` term is currently not applied to `weight`.
This is not a compile error, but it is a behavior caveat.
