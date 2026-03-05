#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

usage() {
    cat <<'USAGE'
Usage: scripts/bot.sh <command> [args]

Commands:
  build [output]              Compile active bot source (default: build/brain)
  test                        Run unit tests
  check                       Syntax-check active targets
  bench                       Run local throughput benchmark
  bundle [output]             Generate single-file submission (default: build/bot_submission.cpp)
  refresh-bot                 Regenerate ./bot.cpp from source
  arena-build [config]        Build arena bot variants (default config: configs/arena_variants.json)
  arena-run [games] [workers] Run arena tournament from existing manifest
  arena [config] [games] [workers]
                              Build + run arena in one command
  auto-improve [g] [p] [e] [games] [workers]
                              Closed-loop self-play parameter optimizer
  promote-best                Build and bundle artifacts from best auto-improve result
  league-add <name> [src]     Add bot to psyleague (default src: brain)
  league-remove <name>        Remove bot from psyleague
  league-run                  Run psyleague
  league-show                 Show psyleague leaderboard
USAGE
}

cmd="${1:-}"
case "$cmd" in
    build)
        out="${2:-build/brain}"
        mkdir -p "$(dirname "$out")"
        g++ -DLOCAL -std=c++20 -O2 src/bot/main.cpp -o "$out"
        ;;
    test)
        make test
        ;;
    check)
        make check
        ;;
    bench)
        make bench
        ;;
    bundle)
        out="${2:-build/bot_submission.cpp}"
        python3 tools/bundle_submission.py brain.cpp --output "$out"
        ;;
    refresh-bot)
        python3 tools/bundle_submission.py brain.cpp --output bot.cpp
        ;;
    arena-build)
        config="${2:-configs/arena_variants.json}"
        python3 tools/arena.py --config "$config" --build-only
        ;;
    arena-run)
        games="${2:-20}"
        workers="${3:-1}"
        python3 tools/arena.py --run-only --games "$games" --workers "$workers"
        ;;
    arena)
        config="${2:-configs/arena_variants.json}"
        games="${3:-20}"
        workers="${4:-1}"
        python3 tools/arena.py --config "$config" --games "$games" --workers "$workers"
        ;;
    auto-improve)
        generations="${2:-4}"
        population="${3:-8}"
        elite="${4:-3}"
        games="${5:-10}"
        workers="${6:-2}"
        python3 tools/auto_improve.py \
            --generations "$generations" \
            --population "$population" \
            --elite "$elite" \
            --games "$games" \
            --workers "$workers"
        ;;
    promote-best)
        python3 tools/promote_best.py
        ;;
    league-add)
        name="${2:-}"
        src="${3:-brain}"
        if [[ -z "$name" ]]; then
            echo "Missing bot name" >&2
            usage
            exit 1
        fi
        psyleague bot add "$name" --src "$src"
        ;;
    league-remove)
        name="${2:-}"
        if [[ -z "$name" ]]; then
            echo "Missing bot name" >&2
            usage
            exit 1
        fi
        psyleague bot remove "$name"
        ;;
    league-run)
        psyleague run
        ;;
    league-show)
        psyleague show
        ;;
    ""|-h|--help|help)
        usage
        ;;
    *)
        echo "Unknown command: $cmd" >&2
        usage
        exit 1
        ;;
esac
