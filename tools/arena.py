#!/usr/bin/env python3
"""
Local tournament runner for Olympbits bot tuning.

Features:
- Build bot variants from compile-time -D defines.
- Run 3-player round-robin matches via referee.jar.
- Produce leaderboard JSON + markdown reports.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import itertools
import json
import os
import random
import re
import subprocess
import sys
import tempfile
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, Iterable, List, Tuple


def now_utc_iso() -> str:
    return datetime.now(timezone.utc).isoformat()


def safe_variant_name(name: str) -> str:
    return re.sub(r"[^a-zA-Z0-9_.-]", "_", name)


def stringify_define_value(value: object) -> str:
    if isinstance(value, bool):
        return "1" if value else "0"
    return str(value)


def parse_java_major(version_text: str) -> int:
    match = re.search(r'version "([^"]+)"', version_text)
    if not match:
        return -1
    version = match.group(1)
    if version.startswith("1."):
        parts = version.split(".")
        if len(parts) >= 2 and parts[1].isdigit():
            return int(parts[1])
        return -1
    major = version.split(".")[0]
    return int(major) if major.isdigit() else -1


def parse_java_version_output(version_text: str) -> Tuple[int, str]:
    major = parse_java_major(version_text)
    headline = version_text.strip().splitlines()[0] if version_text.strip() else "unknown"
    return major, headline


def pick_java_bin(cfg_java_bin: str | None = None) -> str:
    # Priority:
    # 1. Config/CLI explicit path
    # 2. JAVA_BIN env var
    # 3. java-17-openjdk well-known path
    # 4. java on PATH
    if cfg_java_bin:
        return cfg_java_bin
    env_java = os.environ.get("JAVA_BIN")
    if env_java:
        return env_java
    j17 = "/usr/lib/jvm/java-17-openjdk/bin/java"
    if os.path.exists(j17):
        return j17
    return "java"


def require_java_17(java_bin: str) -> None:
    proc = subprocess.run(
        [java_bin, "-version"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )
    version_text = proc.stderr or proc.stdout
    major, headline = parse_java_version_output(version_text)
    if major < 17:
        raise RuntimeError(
            "Java 17+ is required by referee.jar. Detected: "
            f"{headline} (java_bin={java_bin})"
        )


def load_config(path: Path) -> Dict[str, object]:
    with path.open("r", encoding="utf-8") as f:
        cfg = json.load(f)

    if "variants" not in cfg or not isinstance(cfg["variants"], list):
        raise ValueError("Config must contain 'variants' list")

    return cfg


def expand_variants(cfg: Dict[str, object]) -> List[Dict[str, object]]:
    expanded: List[Dict[str, object]] = []
    seen_names = set()

    for raw in cfg.get("variants", []):
        if not isinstance(raw, dict):
            raise ValueError("Each variant must be an object")
        if "name" not in raw:
            raise ValueError("Each variant must define 'name'")
        name = safe_variant_name(str(raw["name"]))
        defines = raw.get("defines", {})
        if not isinstance(defines, dict):
            raise ValueError(f"Variant '{name}' has non-object 'defines'")
        if name in seen_names:
            raise ValueError(f"Duplicate variant name: {name}")
        seen_names.add(name)
        expanded.append({"name": name, "defines": dict(defines)})

    grid = cfg.get("grid")
    if grid is not None:
        if not isinstance(grid, dict) or not grid:
            raise ValueError("'grid' must be a non-empty object when present")
        keys = sorted(grid.keys())
        values_lists: List[List[object]] = []
        for key in keys:
            values = grid[key]
            if not isinstance(values, list) or not values:
                raise ValueError(f"Grid parameter '{key}' must be a non-empty list")
            values_lists.append(values)

        prefix = str(cfg.get("grid_prefix", "grid"))
        for combo in itertools.product(*values_lists):
            defines = {k: combo[i] for i, k in enumerate(keys)}
            suffix_parts = []
            for k, v in defines.items():
                value_str = str(v).replace(".", "p").replace("-", "m")
                suffix_parts.append(f"{k}_{value_str}")
            name = safe_variant_name(prefix + "__" + "__".join(suffix_parts))
            if name in seen_names:
                raise ValueError(f"Grid variant name collision: {name}")
            seen_names.add(name)
            expanded.append({"name": name, "defines": defines})

    return expanded


def compile_variant(
    source: Path,
    out_exe: Path,
    cxx: str,
    cxxflags: List[str],
    defines: Dict[str, object],
) -> Dict[str, object]:
    out_exe.parent.mkdir(parents=True, exist_ok=True)

    cmd = [cxx] + cxxflags
    for key in sorted(defines.keys()):
        value = stringify_define_value(defines[key])
        cmd.append(f"-D{key}={value}")
    cmd += [str(source), "-o", str(out_exe)]

    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=False)
    if proc.returncode != 0:
        raise RuntimeError(
            "Compilation failed for "
            f"{out_exe.name}\nCMD: {' '.join(cmd)}\n{proc.stderr}"
        )

    return {"cmd": cmd}


def build_manifest(
    cfg: Dict[str, object],
    cfg_path: Path,
    manifest_path: Path,
    root_dir: Path,
) -> Dict[str, object]:
    source = root_dir / str(cfg.get("source", "brain.cpp"))
    cxx = str(cfg.get("cxx", "g++"))
    cxxflags = [str(x) for x in cfg.get("cxxflags", ["-std=c++20", "-O2", "-DLOCAL", "-DPSYLEAGUE"])]
    build_dir = root_dir / str(cfg.get("build_dir", "arena/build"))
    referee = root_dir / str(cfg.get("referee", "referee.jar"))
    java_bin = pick_java_bin(cfg.get("java_bin"))

    require_java_17(java_bin)

    variants = expand_variants(cfg)
    bots: Dict[str, Dict[str, object]] = {}

    print(f"[build] source={source}")
    print(f"[build] variants={len(variants)}")
    for idx, variant in enumerate(variants, start=1):
        name = str(variant["name"])
        defines = dict(variant["defines"])
        out_exe = build_dir / f"{name}.exe"
        print(f"[build] ({idx}/{len(variants)}) {name}")
        compile_meta = compile_variant(source, out_exe, cxx, cxxflags, defines)
        bots[name] = {
            "name": name,
            "exe": str(out_exe),
            "defines": {k: stringify_define_value(v) for k, v in sorted(defines.items())},
            "compile_cmd": compile_meta["cmd"],
        }

    manifest = {
        "created_at_utc": now_utc_iso(),
        "config_path": str(cfg_path),
        "source": str(source),
        "referee": str(referee),
        "java_bin": java_bin,
        "bots": bots,
    }

    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    with manifest_path.open("w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2)
    print(f"[build] wrote manifest: {manifest_path}")
    return manifest


def compute_ranks(scores: List[int]) -> List[int]:
    # Rank 0 = best (higher score is better), same semantics as play_game.py.
    return [sum(int(s < other) for other in scores) for s in scores]


def run_referee_game(java_bin: str, referee: str, lineup_execs: List[str], seed: int) -> Dict[str, object]:
    fd, log_file = tempfile.mkstemp(prefix="arena_log_", suffix=".json")
    os.close(fd)

    cmd = [java_bin, "-jar", referee]
    for i, exe in enumerate(lineup_execs, start=1):
        cmd += [f"-p{i}", exe]
    cmd += ["-d", f"seed={seed}", "-l", log_file]

    t0 = time.perf_counter()
    proc = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True, check=False)
    elapsed_ms = int((time.perf_counter() - t0) * 1000)

    try:
        if proc.returncode != 0:
            return {
                "ok": False,
                "elapsed_ms": elapsed_ms,
                "seed": seed,
                "error": f"Referee exited with code {proc.returncode}: {proc.stderr[:800]}",
            }

        if not os.path.exists(log_file) or os.path.getsize(log_file) == 0:
            return {
                "ok": False,
                "elapsed_ms": elapsed_ms,
                "seed": seed,
                "error": "Referee log is empty (check Java version and referee compatibility).",
            }

        with open(log_file, "r", encoding="utf-8") as f:
            json_log = json.load(f)

        n_players = len(lineup_execs)
        scores = [int(json_log["scores"][str(i)]) for i in range(n_players)]
        ranks = compute_ranks(scores)
        errors = [int(score < 0) for score in scores]
        return {
            "ok": True,
            "elapsed_ms": elapsed_ms,
            "seed": seed,
            "scores": scores,
            "ranks": ranks,
            "errors": errors,
        }
    finally:
        if os.path.exists(log_file):
            os.remove(log_file)


def _worker_run_game(task: Dict[str, object]) -> Dict[str, object]:
    game = run_referee_game(
        java_bin=str(task["java_bin"]),
        referee=str(task["referee"]),
        lineup_execs=list(task["lineup_execs"]),
        seed=int(task["seed"]),
    )
    game["lineup_names"] = list(task["lineup_names"])
    return game


def generate_lineups(bot_names: List[str], seat_permutations: bool) -> List[Tuple[str, str, str]]:
    if len(bot_names) < 3:
        raise ValueError("At least 3 bot variants are required for a 3-player arena")
    lineups: List[Tuple[str, str, str]] = []
    for combo in itertools.combinations(bot_names, 3):
        if seat_permutations:
            for perm in itertools.permutations(combo, 3):
                lineups.append((perm[0], perm[1], perm[2]))
        else:
            lineups.append((combo[0], combo[1], combo[2]))
    return lineups


def create_tasks(
    manifest: Dict[str, object],
    games_per_lineup: int,
    seat_permutations: bool,
    seed: int,
) -> Tuple[List[Tuple[str, str, str]], Iterable[Dict[str, object]]]:
    bots = manifest["bots"]
    bot_names = sorted(bots.keys())
    lineups = generate_lineups(bot_names, seat_permutations)

    rng = random.Random(seed)
    referee = str(manifest["referee"])
    java_bin = str(manifest.get("java_bin", pick_java_bin(None)))

    def iterator() -> Iterable[Dict[str, object]]:
        for lineup in lineups:
            lineup_execs = [str(bots[name]["exe"]) for name in lineup]
            for _ in range(games_per_lineup):
                yield {
                    "java_bin": java_bin,
                    "referee": referee,
                    "lineup_names": lineup,
                    "lineup_execs": lineup_execs,
                    "seed": rng.randrange(0, 2**31),
                }

    return lineups, iterator()


def init_bot_stats(bot_names: List[str]) -> Dict[str, Dict[str, float]]:
    stats: Dict[str, Dict[str, float]] = {}
    for name in bot_names:
        stats[name] = {
            "games": 0.0,
            "firsts": 0.0,
            "seconds": 0.0,
            "thirds": 0.0,
            "rank_sum": 0.0,
            "points": 0.0,
            "errors": 0.0,
            "wall_ms_sum": 0.0,
        }
    return stats


def points_from_rank(rank: int) -> int:
    if rank == 0:
        return 3
    if rank == 1:
        return 1
    return 0


def run_tournament(
    manifest: Dict[str, object],
    out_dir: Path,
    games_per_lineup: int,
    workers: int,
    seat_permutations: bool,
    seed: int,
) -> Dict[str, object]:
    require_java_17(str(manifest.get("java_bin", pick_java_bin(None))))

    bots = manifest["bots"]
    bot_names = sorted(bots.keys())
    bot_stats = init_bot_stats(bot_names)

    pairwise: Dict[str, Dict[str, object]] = {}
    failures: List[Dict[str, object]] = []

    lineups, tasks_iter = create_tasks(manifest, games_per_lineup, seat_permutations, seed)
    total_games = len(lineups) * games_per_lineup
    print(f"[run] bots={len(bot_names)} lineups={len(lineups)} total_games={total_games} workers={workers}")

    completed = 0
    t0 = time.perf_counter()

    def consume_game(game: Dict[str, object]) -> None:
        nonlocal completed
        completed += 1
        if completed % 25 == 0 or completed == total_games:
            elapsed = time.perf_counter() - t0
            gps = completed / elapsed if elapsed > 0 else 0.0
            print(f"[run] {completed}/{total_games} games ({gps:.2f} games/s)")

        lineup = list(game["lineup_names"])
        if not game["ok"]:
            failures.append(
                {
                    "lineup": lineup,
                    "seed": game.get("seed"),
                    "error": game.get("error", "unknown"),
                }
            )
            return

        ranks = list(game["ranks"])
        scores = list(game["scores"])
        errors = list(game["errors"])
        elapsed_ms = float(game["elapsed_ms"])

        for i, name in enumerate(lineup):
            stat = bot_stats[name]
            rank = int(ranks[i])
            stat["games"] += 1
            stat["rank_sum"] += rank
            stat["points"] += points_from_rank(rank)
            stat["errors"] += int(errors[i])
            stat["wall_ms_sum"] += elapsed_ms
            if rank == 0:
                stat["firsts"] += 1
            elif rank == 1:
                stat["seconds"] += 1
            else:
                stat["thirds"] += 1

        for i, j in ((0, 1), (0, 2), (1, 2)):
            a = lineup[i]
            b = lineup[j]
            key = "|".join(sorted([a, b]))
            if key not in pairwise:
                na, nb = sorted([a, b])
                pairwise[key] = {"a": na, "b": nb, "a_wins": 0, "b_wins": 0, "ties": 0}
            rec = pairwise[key]
            if scores[i] == scores[j]:
                rec["ties"] += 1
            elif scores[i] > scores[j]:
                if a == rec["a"]:
                    rec["a_wins"] += 1
                else:
                    rec["b_wins"] += 1
            else:
                if b == rec["a"]:
                    rec["a_wins"] += 1
                else:
                    rec["b_wins"] += 1

    if workers <= 1:
        for task in tasks_iter:
            consume_game(_worker_run_game(task))
    else:
        # Thread pool is sandbox-safe and still scales because each job waits on subprocesses.
        with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
            for game in pool.map(_worker_run_game, tasks_iter):
                consume_game(game)

    leaderboard = []
    for name in bot_names:
        stat = bot_stats[name]
        games = max(1.0, stat["games"])
        error_rate = stat["errors"] / games
        points_per_game = stat["points"] / games
        entry = {
            "name": name,
            "games": int(stat["games"]),
            "firsts": int(stat["firsts"]),
            "seconds": int(stat["seconds"]),
            "thirds": int(stat["thirds"]),
            "errors": int(stat["errors"]),
            "error_rate": error_rate,
            "winrate": stat["firsts"] / games,
            "top2_rate": (stat["firsts"] + stat["seconds"]) / games,
            "avg_rank": stat["rank_sum"] / games,
            "points_per_game": points_per_game,
            # Penalize unstable bots so optimizer avoids "good rank but many crashes/timeouts".
            "fitness": points_per_game - 1.5 * error_rate,
            "avg_game_ms": stat["wall_ms_sum"] / games,
            "defines": bots[name].get("defines", {}),
        }
        leaderboard.append(entry)

    leaderboard.sort(
        key=lambda e: (
            e["fitness"],
            e["points_per_game"],
            e["winrate"],
            -e["avg_rank"],
            -e["errors"],
        ),
        reverse=True,
    )

    elapsed_total_ms = int((time.perf_counter() - t0) * 1000)
    result = {
        "created_at_utc": now_utc_iso(),
        "seed": seed,
        "games_per_lineup": games_per_lineup,
        "seat_permutations": seat_permutations,
        "workers": workers,
        "total_games_scheduled": total_games,
        "total_games_completed": completed - len(failures),
        "total_failures": len(failures),
        "elapsed_total_ms": elapsed_total_ms,
        "leaderboard": leaderboard,
        "pairwise": list(pairwise.values()),
        "failures": failures[:200],
    }

    out_dir.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    json_path = out_dir / f"arena_result_{stamp}.json"
    md_path = out_dir / f"arena_result_{stamp}.md"
    latest_json = out_dir / "latest.json"
    latest_md = out_dir / "latest.md"

    with json_path.open("w", encoding="utf-8") as f:
        json.dump(result, f, indent=2)
    with latest_json.open("w", encoding="utf-8") as f:
        json.dump(result, f, indent=2)

    with md_path.open("w", encoding="utf-8") as f:
        f.write(render_markdown_report(result))
    with latest_md.open("w", encoding="utf-8") as f:
        f.write(render_markdown_report(result))

    print(f"[run] wrote: {json_path}")
    print(f"[run] wrote: {md_path}")
    return result


def render_markdown_report(result: Dict[str, object]) -> str:
    lines = []
    lines.append("# Arena Leaderboard")
    lines.append("")
    lines.append(f"- Created (UTC): {result['created_at_utc']}")
    lines.append(f"- Games per lineup: {result['games_per_lineup']}")
    lines.append(f"- Seat permutations: {result['seat_permutations']}")
    lines.append(f"- Workers: {result['workers']}")
    lines.append(f"- Scheduled games: {result['total_games_scheduled']}")
    lines.append(f"- Completed games: {result['total_games_completed']}")
    lines.append(f"- Failures: {result['total_failures']}")
    lines.append(f"- Total wall time (ms): {result['elapsed_total_ms']}")
    lines.append("")
    lines.append("| POS | BOT | G | FIT | P/G | WR | TOP2 | AVG_RANK | ERR% |")
    lines.append("|---:|:---|---:|---:|---:|---:|---:|---:|---:|")
    for pos, entry in enumerate(result["leaderboard"], start=1):
        lines.append(
            f"| {pos} | {entry['name']} | {entry['games']} | "
            f"{entry['fitness']:.4f} | {entry['points_per_game']:.4f} | {entry['winrate']:.4f} | "
            f"{entry['top2_rate']:.4f} | {entry['avg_rank']:.4f} | {entry['error_rate']:.2%} |"
        )
    lines.append("")
    lines.append("## Top Variant Defines")
    lines.append("")
    for entry in result["leaderboard"][:10]:
        lines.append(f"- `{entry['name']}`: `{entry.get('defines', {})}`")
    return "\n".join(lines) + "\n"


def parse_args(argv: List[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build and run local Olympbits arena")
    parser.add_argument("--config", default="configs/arena_variants.json", help="Path to arena config JSON")
    parser.add_argument("--manifest", default="arena/build/manifest.json", help="Path to build manifest JSON")
    parser.add_argument("--results-dir", default="arena/results", help="Directory for tournament reports")
    parser.add_argument("--games", type=int, default=20, help="Games per lineup")
    parser.add_argument("--workers", type=int, default=max(1, (os.cpu_count() or 1)), help="Parallel game workers")
    parser.add_argument("--seed", type=int, default=None, help="PRNG seed for matchup generation")
    parser.add_argument(
        "--no-seat-permutations",
        action="store_true",
        help="Disable seat-order permutations (faster, but can bias results)",
    )
    parser.add_argument("--build-only", action="store_true", help="Only compile variants")
    parser.add_argument("--run-only", action="store_true", help="Only run tournament from existing manifest")
    return parser.parse_args(argv)


def main(argv: List[str]) -> int:
    args = parse_args(argv)
    if args.build_only and args.run_only:
        raise SystemExit("Cannot use --build-only and --run-only together")

    root_dir = Path(__file__).resolve().parents[1]
    cfg_path = (root_dir / args.config).resolve()
    manifest_path = (root_dir / args.manifest).resolve()
    results_dir = (root_dir / args.results_dir).resolve()

    do_build = not args.run_only
    do_run = not args.build_only
    seed = args.seed if args.seed is not None else random.randrange(0, 2**31)

    manifest = None
    if do_build:
        cfg = load_config(cfg_path)
        manifest = build_manifest(cfg, cfg_path, manifest_path, root_dir)
    else:
        with manifest_path.open("r", encoding="utf-8") as f:
            manifest = json.load(f)

    if do_run:
        run_tournament(
            manifest=manifest,
            out_dir=results_dir,
            games_per_lineup=max(1, args.games),
            workers=max(1, args.workers),
            seat_permutations=not args.no_seat_permutations,
            seed=seed,
        )

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main(sys.argv[1:]))
    except Exception as exc:  # pragma: no cover - CLI guard
        print(f"[error] {exc}", file=sys.stderr)
        raise SystemExit(1)
