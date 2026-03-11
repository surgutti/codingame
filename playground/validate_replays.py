#!/usr/bin/env python3

from __future__ import annotations

import argparse
import gzip
import json
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path


BASE_DIR = Path(__file__).resolve().parent
REPLAY_DIR = BASE_DIR / "replays"
DEFAULT_DRIVER = BASE_DIR.parent / "WinterChallenge2026-Exotec" / "cpp" / "build" / "referee_replay_driver"
TIMEOUT_MARKER = "@@TIMEOUT@@"


@dataclass
class TurnExpectation:
    command_0: str
    command_1: str
    exec_0: int
    exec_1: int
    duration: int
    graphics: str


@dataclass
class ReplayExpectation:
    replay_id: str
    seed: int
    league: int
    global_graphics: str
    initial_frame_graphics: str
    initial_exec_0: int
    initial_exec_1: int
    turns: list[TurnExpectation]
    score_0: int
    score_1: int


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--driver", type=Path, default=DEFAULT_DRIVER)
    parser.add_argument("--replay", type=Path)
    parser.add_argument("--limit", type=int)
    parser.add_argument("--fail-fast", action="store_true")
    parser.add_argument("--strict", action="store_true")
    return parser.parse_args()


def load_json_gz(path: Path) -> dict:
    with gzip.open(path, "rt", encoding="utf-8") as handle:
        return json.load(handle)


def split_view(view: str) -> dict:
    _, _, payload = view.partition("\n")
    payload = payload.strip()
    if not payload:
        raise ValueError("missing JSON payload in keyframe view")
    return json.loads(payload)


def extract_exec_times(graphics: str) -> tuple[int, int]:
    parts = graphics.split("|")
    idx = 0
    event_count = int(parts[idx])
    idx += 1
    idx += event_count * 4

    exec_times: list[int] = []
    for _ in range(2):
        exec_times.append(int(parts[idx]))
        idx += 1
        mark_count = int(parts[idx])
        idx += 1 + mark_count

    return exec_times[0], exec_times[1]


def timeout_summary_for_agent(summary: str, agent_id: int) -> bool:
    return f"${agent_id} has not provided 1 lines in time" in summary


def clean_stdout(frame: dict, next_frame: dict | None) -> str:
    agent_id = frame.get("agentId")
    if timeout_summary_for_agent(frame.get("summary", ""), agent_id):
        return TIMEOUT_MARKER
    if next_frame is not None and next_frame.get("keyframe") and timeout_summary_for_agent(next_frame.get("summary", ""), agent_id):
        return TIMEOUT_MARKER
    if "stdout" not in frame:
        return TIMEOUT_MARKER
    stdout = frame["stdout"].removesuffix("\n")
    return stdout


def parse_replay(path: Path) -> ReplayExpectation:
    data = load_json_gz(path)
    replay_id = path.stem.replace(".json", "")

    initial = split_view(data["frames"][0]["view"])
    global_graphics = initial["global"]["graphics"]
    initial_frame_graphics = initial["frame"]["graphics"]
    init_exec_0, init_exec_1 = extract_exec_times(initial_frame_graphics)

    turns: list[TurnExpectation] = []
    pending = {0: None, 1: None}
    last_duration = 1000
    frames = data["frames"]

    for index, frame in enumerate(frames[1:], start=1):
        agent_id = frame.get("agentId")
        if agent_id not in (0, 1):
            continue

        next_frame = frames[index + 1] if index + 1 < len(frames) else None
        pending[agent_id] = clean_stdout(frame, next_frame)

        if not frame.get("keyframe"):
            continue

        payload = split_view(frame["view"])
        if "duration" in payload:
            last_duration = int(payload["duration"])
        exec_0, exec_1 = extract_exec_times(payload["graphics"])
        turns.append(
            TurnExpectation(
                command_0=pending[0] if pending[0] is not None else TIMEOUT_MARKER,
                command_1=pending[1] if pending[1] is not None else TIMEOUT_MARKER,
                exec_0=exec_0,
                exec_1=exec_1,
                duration=last_duration,
                graphics=payload["graphics"],
            )
        )
        pending = {0: None, 1: None}

    seed_line = data["refereeInput"].strip()
    if not seed_line.startswith("seed="):
        raise ValueError(f"{path.name}: unexpected referee input {seed_line!r}")
    seed = int(seed_line.split("=", 1)[1])

    scores = data["scores"]
    return ReplayExpectation(
        replay_id=replay_id,
        seed=seed,
        league=5,
        global_graphics=global_graphics,
        initial_frame_graphics=initial_frame_graphics,
        initial_exec_0=init_exec_0,
        initial_exec_1=init_exec_1,
        turns=turns,
        score_0=int(scores[0]),
        score_1=int(scores[1]),
    )


def first_token_diff(expected: str, actual: str) -> str:
    exp = expected.split("|")
    act = actual.split("|")
    limit = min(len(exp), len(act))
    for idx in range(limit):
        if exp[idx] != act[idx]:
            return f"token {idx}: expected {exp[idx]!r}, got {act[idx]!r}"
    if len(exp) != len(act):
        return f"token count: expected {len(exp)}, got {len(act)}"
    return "no diff"


def run_driver(driver: Path, replay: ReplayExpectation) -> tuple[str, str, list[tuple[int, str]], tuple[int, int]]:
    lines = [
        str(replay.seed),
        str(replay.league),
        str(len(replay.turns)),
        str(replay.initial_exec_0),
        str(replay.initial_exec_1),
    ]
    for turn in replay.turns:
        lines.extend(
            [
                str(turn.exec_0),
                str(turn.exec_1),
                turn.command_0,
                turn.command_1,
            ]
        )
    payload = "\n".join(lines) + "\n"
    completed = subprocess.run(
        [str(driver)],
        input=payload,
        text=True,
        capture_output=True,
        check=False,
    )
    if completed.returncode != 0:
        raise RuntimeError(completed.stderr.strip() or "driver failed")

    output_lines = completed.stdout.splitlines()
    expected_lines = 2 + 2 * len(replay.turns) + 1
    if len(output_lines) != expected_lines:
        raise RuntimeError(f"unexpected driver output line count {len(output_lines)} != {expected_lines}")

    global_graphics = output_lines[0]
    initial_frame_graphics = output_lines[1]

    turns: list[tuple[int, str]] = []
    idx = 2
    for _ in replay.turns:
        turns.append((int(output_lines[idx]), output_lines[idx + 1]))
        idx += 2

    score_0, score_1 = map(int, output_lines[idx].split())
    return global_graphics, initial_frame_graphics, turns, (score_0, score_1)


def infer_league(driver: Path, replay: ReplayExpectation) -> int | None:
    probe = ReplayExpectation(
        replay_id=replay.replay_id,
        seed=replay.seed,
        league=5,
        global_graphics=replay.global_graphics,
        initial_frame_graphics=replay.initial_frame_graphics,
        initial_exec_0=replay.initial_exec_0,
        initial_exec_1=replay.initial_exec_1,
        turns=[],
        score_0=0,
        score_1=0,
    )
    for candidate in (1, 2, 3, 5):
        probe.league = candidate
        actual_global, _, _, _ = run_driver(driver, probe)
        if actual_global == replay.global_graphics:
            return candidate
    return None


def validate_replay(driver: Path, path: Path, strict: bool) -> tuple[str, str]:
    replay = parse_replay(path)
    inferred = infer_league(driver, replay)
    if inferred is None:
        if strict:
            return "fail", f"{path.name}: replay does not match the current WinterChallenge2026-Exotec Java source"
        return "skip", f"{path.name}: skipped, replay does not match the current WinterChallenge2026-Exotec Java source"
    replay.league = inferred
    actual_global, actual_initial, actual_turns, actual_scores = run_driver(driver, replay)

    if actual_global != replay.global_graphics:
        return "fail", f"{path.name}: global mismatch, {first_token_diff(replay.global_graphics, actual_global)}"

    if actual_initial != replay.initial_frame_graphics:
        return "fail", f"{path.name}: initial frame mismatch, {first_token_diff(replay.initial_frame_graphics, actual_initial)}"

    for turn_idx, expected_turn in enumerate(replay.turns):
        actual_duration, actual_graphics = actual_turns[turn_idx]
        if actual_duration != expected_turn.duration:
            return "fail", (
                f"{path.name}: turn {turn_idx} duration mismatch, "
                f"expected {expected_turn.duration}, got {actual_duration}"
            )
        if actual_graphics != expected_turn.graphics:
            return "fail", (
                f"{path.name}: turn {turn_idx} graphics mismatch, "
                f"{first_token_diff(expected_turn.graphics, actual_graphics)}"
            )

    if actual_scores != (replay.score_0, replay.score_1):
        return "fail", (
            f"{path.name}: score mismatch, expected {(replay.score_0, replay.score_1)}, "
            f"got {actual_scores}"
        )

    return "ok", f"{path.name}: ok"


def iter_replays(single: Path | None, limit: int | None) -> list[Path]:
    if single is not None:
        return [single]
    paths = sorted(REPLAY_DIR.glob("*.json.gz"))
    if limit is not None:
        paths = paths[:limit]
    return paths


def main() -> int:
    args = parse_args()
    if not args.driver.exists():
        print(f"missing driver: {args.driver}", file=sys.stderr)
        return 1

    paths = iter_replays(args.replay, args.limit)
    if not paths:
        print("no replays found", file=sys.stderr)
        return 1

    start = time.perf_counter()
    ok_count = 0
    skip_count = 0
    failures: list[str] = []

    for path in paths:
        try:
            status, message = validate_replay(args.driver, path, args.strict)
        except Exception as exc:
            status = "fail"
            message = f"{path.name}: validator error: {exc}"

        if status == "ok":
            ok_count += 1
            continue
        if status == "skip":
            skip_count += 1
            print(message, file=sys.stderr)
            continue
        failures.append(message)
        print(message, file=sys.stderr)
        if args.fail_fast:
            break

    elapsed = time.perf_counter() - start
    print(f"validated {ok_count}/{len(paths)} replays in {elapsed:.2f}s, skipped {skip_count}")
    if failures:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
