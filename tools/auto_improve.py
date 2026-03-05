#!/usr/bin/env python3
"""
Closed-loop self-play optimizer for Olympbits bot parameters.

Runs repeated arena tournaments, keeps top performers, mutates them, and
continues for N generations.
"""

from __future__ import annotations

import argparse
import json
import random
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, List, Tuple

import arena as arena_lib


@dataclass(frozen=True)
class ParamSpec:
    name: str
    kind: str  # "int", "float", "bool"
    low: float
    high: float
    step: float


DEFAULT_SPECS: List[ParamSpec] = [
    ParamSpec("BOT_TIMEOUT_SCALE_PCT", "int", 80, 240, 5),
    ParamSpec("BOT_UCB_C", "float", 0.20, 0.80, 0.01),
    ParamSpec("BOT_RANDOM_PHASE_RATIO", "float", 0.04, 0.35, 0.01),
    ParamSpec("BOT_TIMER_CHECK_MASK", "int", 1, 7, 1),
    ParamSpec("BOT_REGENERATE_ENDED_GAMES", "bool", 0, 1, 1),
    ParamSpec("BOT_WEIGHT_MODE", "bool", 0, 1, 1),
    ParamSpec("BOT_ROLLOUT_GREEDY_RANDOM_PCT", "int", 0, 20, 1),
]


def canonical_defines(defines: Dict[str, object]) -> Tuple[Tuple[str, str], ...]:
    return tuple((k, str(defines[k])) for k in sorted(defines.keys()))


def sanitize_define_value(spec: ParamSpec, value: float) -> object:
    if spec.kind == "bool":
        return 1 if value >= 0.5 else 0
    if spec.kind == "int":
        v = int(round(value))
        v = max(int(spec.low), min(int(spec.high), v))
        return v
    v = max(spec.low, min(spec.high, value))
    # Keep 3 decimals to avoid noisy variant strings.
    return round(v, 3)


def default_seed_population() -> List[Dict[str, object]]:
    return [
        {
            "BOT_TIMEOUT_SCALE_PCT": 100,
            "BOT_UCB_C": 0.40,
            "BOT_RANDOM_PHASE_RATIO": 0.12,
            "BOT_TIMER_CHECK_MASK": 3,
            "BOT_REGENERATE_ENDED_GAMES": 1,
            "BOT_WEIGHT_MODE": 0,
            "BOT_ROLLOUT_GREEDY_RANDOM_PCT": 0,
        },
        {
            "BOT_TIMEOUT_SCALE_PCT": 150,
            "BOT_UCB_C": 0.40,
            "BOT_RANDOM_PHASE_RATIO": 0.12,
            "BOT_TIMER_CHECK_MASK": 3,
            "BOT_REGENERATE_ENDED_GAMES": 1,
            "BOT_WEIGHT_MODE": 0,
            "BOT_ROLLOUT_GREEDY_RANDOM_PCT": 0,
        },
        {
            "BOT_TIMEOUT_SCALE_PCT": 200,
            "BOT_UCB_C": 0.40,
            "BOT_RANDOM_PHASE_RATIO": 0.12,
            "BOT_TIMER_CHECK_MASK": 3,
            "BOT_REGENERATE_ENDED_GAMES": 1,
            "BOT_WEIGHT_MODE": 0,
            "BOT_ROLLOUT_GREEDY_RANDOM_PCT": 0,
        },
        {
            "BOT_TIMEOUT_SCALE_PCT": 100,
            "BOT_UCB_C": 0.40,
            "BOT_RANDOM_PHASE_RATIO": 0.12,
            "BOT_TIMER_CHECK_MASK": 3,
            "BOT_REGENERATE_ENDED_GAMES": 0,  # post-mortem style: no regeneration
            "BOT_WEIGHT_MODE": 0,
            "BOT_ROLLOUT_GREEDY_RANDOM_PCT": 0,
        },
        {
            "BOT_TIMEOUT_SCALE_PCT": 100,
            "BOT_UCB_C": 0.40,
            "BOT_RANDOM_PHASE_RATIO": 0.12,
            "BOT_TIMER_CHECK_MASK": 3,
            "BOT_REGENERATE_ENDED_GAMES": 1,
            "BOT_WEIGHT_MODE": 1,  # post-mortem style: inverse-score weighting
            "BOT_ROLLOUT_GREEDY_RANDOM_PCT": 0,
        },
    ]


def mutate(defines: Dict[str, object], specs: List[ParamSpec], rng: random.Random) -> Dict[str, object]:
    child = dict(defines)
    n_changes = 1 + (1 if rng.random() < 0.35 else 0)
    for _ in range(n_changes):
        spec = rng.choice(specs)
        key = spec.name
        base = float(child.get(key, 0))
        if spec.kind == "bool":
            child[key] = 1 - int(base >= 0.5)
            continue
        delta_steps = rng.choice([-3, -2, -1, 1, 2, 3])
        delta = delta_steps * spec.step
        if spec.kind == "float":
            # Slightly wider exploration for continuous params.
            delta *= rng.uniform(0.5, 1.5)
        child[key] = sanitize_define_value(spec, base + delta)
    return child


def random_variant(specs: List[ParamSpec], rng: random.Random) -> Dict[str, object]:
    defines: Dict[str, object] = {}
    for spec in specs:
        if spec.kind == "bool":
            defines[spec.name] = rng.choice([0, 1])
        elif spec.kind == "int":
            lo = int(spec.low)
            hi = int(spec.high)
            step = int(spec.step)
            values = list(range(lo, hi + 1, step))
            defines[spec.name] = rng.choice(values)
        else:
            steps = int(round((spec.high - spec.low) / spec.step))
            idx = rng.randint(0, max(0, steps))
            defines[spec.name] = round(spec.low + idx * spec.step, 3)
    return defines


def make_named_variants(population: List[Dict[str, object]], generation: int) -> List[Dict[str, object]]:
    variants = []
    for i, defines in enumerate(population):
        short = []
        for key in ["BOT_TIMEOUT_SCALE_PCT", "BOT_UCB_C", "BOT_RANDOM_PHASE_RATIO", "BOT_WEIGHT_MODE", "BOT_REGENERATE_ENDED_GAMES"]:
            if key in defines:
                value = str(defines[key]).replace(".", "p")
                short.append(f"{key.split('_')[-1].lower()}{value}")
        name = f"g{generation:02d}_v{i:02d}_" + "_".join(short)
        variants.append({"name": arena_lib.safe_variant_name(name), "defines": defines})
    return variants


def run_generation(
    root_dir: Path,
    generation: int,
    population: List[Dict[str, object]],
    games: int,
    workers: int,
    seat_permutations: bool,
    seed: int,
) -> Dict[str, object]:
    cfg = {
        "source": "brain.cpp",
        "cxx": "g++",
        "cxxflags": ["-std=c++20", "-O2", "-DLOCAL", "-DPSYLEAGUE"],
        "referee": "referee.jar",
        "build_dir": f"arena/auto/gen_{generation:02d}/build",
        "variants": make_named_variants(population, generation),
    }

    cfg_path = root_dir / f"arena/auto/gen_{generation:02d}/config.json"
    manifest_path = root_dir / f"arena/auto/gen_{generation:02d}/manifest.json"
    results_dir = root_dir / f"arena/auto/gen_{generation:02d}/results"
    cfg_path.parent.mkdir(parents=True, exist_ok=True)
    with cfg_path.open("w", encoding="utf-8") as f:
        json.dump(cfg, f, indent=2)

    manifest = arena_lib.build_manifest(cfg, cfg_path, manifest_path, root_dir)
    result = arena_lib.run_tournament(
        manifest=manifest,
        out_dir=results_dir,
        games_per_lineup=max(1, games),
        workers=max(1, workers),
        seat_permutations=seat_permutations,
        seed=seed,
    )
    result["generation"] = generation
    result["config_path"] = str(cfg_path)
    result["manifest_path"] = str(manifest_path)
    return result


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Automatic closed-loop arena optimizer")
    parser.add_argument("--generations", type=int, default=4, help="Number of optimization generations")
    parser.add_argument("--population", type=int, default=8, help="Variants evaluated per generation")
    parser.add_argument("--elite", type=int, default=3, help="Top variants kept per generation")
    parser.add_argument("--games", type=int, default=10, help="Games per lineup")
    parser.add_argument("--workers", type=int, default=2, help="Parallel arena workers")
    parser.add_argument("--seed", type=int, default=1337, help="Optimizer random seed")
    parser.add_argument(
        "--no-seat-permutations",
        action="store_true",
        help="Disable seat permutations for faster but less robust ranking",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.population < 3:
        raise SystemExit("population must be >= 3")
    if args.elite < 1:
        raise SystemExit("elite must be >= 1")

    root_dir = Path(__file__).resolve().parents[1]
    rng = random.Random(args.seed)
    specs = DEFAULT_SPECS

    population = default_seed_population()
    while len(population) < args.population:
        population.append(random_variant(specs, rng))
    population = population[: args.population]

    global_best = None
    history = []

    for generation in range(args.generations):
        print(
            f"[auto] generation={generation} population={len(population)} "
            f"games={args.games} workers={args.workers}"
        )
        gen_seed = rng.randrange(0, 2**31)
        result = run_generation(
            root_dir=root_dir,
            generation=generation,
            population=population,
            games=args.games,
            workers=args.workers,
            seat_permutations=not args.no_seat_permutations,
            seed=gen_seed,
        )
        history.append(
            {
                "generation": generation,
                "seed": gen_seed,
                "leaderboard": result["leaderboard"],
                "result_path": str(root_dir / f"arena/auto/gen_{generation:02d}/results/latest.json"),
            }
        )

        leaderboard = result["leaderboard"]
        if not leaderboard:
            raise RuntimeError(f"No leaderboard entries for generation {generation}")

        if global_best is None or leaderboard[0].get("fitness", 0.0) > global_best.get("fitness", 0.0):
            global_best = leaderboard[0]
            print(
                f"[auto] new_best generation={generation} "
                f"name={global_best['name']} fitness={global_best.get('fitness', 0.0):.4f} "
                f"ppg={global_best['points_per_game']:.4f}"
            )

        elite_count = min(args.elite, len(leaderboard))
        elites = [entry["defines"] for entry in leaderboard[:elite_count]]

        next_population: List[Dict[str, object]] = [dict(e) for e in elites]
        seen = {canonical_defines(e) for e in next_population}

        while len(next_population) < args.population:
            if rng.random() < 0.75 and elites:
                parent = dict(rng.choice(elites))
                candidate = mutate(parent, specs, rng)
            else:
                candidate = random_variant(specs, rng)
            key = canonical_defines(candidate)
            if key in seen:
                continue
            seen.add(key)
            next_population.append(candidate)

        population = next_population

    summary = {
        "created_at": datetime.now(timezone.utc).isoformat(),
        "seed": args.seed,
        "generations": args.generations,
        "population": args.population,
        "elite": args.elite,
        "games": args.games,
        "workers": args.workers,
        "best": global_best,
        "history": history,
    }

    out_dir = root_dir / "arena/auto"
    out_dir.mkdir(parents=True, exist_ok=True)
    summary_path = out_dir / "latest_summary.json"
    with summary_path.open("w", encoding="utf-8") as f:
        json.dump(summary, f, indent=2)

    if global_best is not None:
        best_cfg = {
            "source": "brain.cpp",
            "cxx": "g++",
            "cxxflags": ["-std=c++20", "-O2", "-DLOCAL", "-DPSYLEAGUE"],
            "referee": "referee.jar",
            "build_dir": "arena/best/build",
            "variants": [{"name": "best_auto", "defines": global_best["defines"]}],
        }
        best_cfg_path = out_dir / "best_config.json"
        with best_cfg_path.open("w", encoding="utf-8") as f:
            json.dump(best_cfg, f, indent=2)
        print(f"[auto] best defines: {global_best['defines']}")
        print(f"[auto] best config: {best_cfg_path}")

    print(f"[auto] summary: {summary_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
