#!/usr/bin/env python3
"""Validate the one-turn simulator harness against local replay files."""

from __future__ import annotations

import argparse
import gzip
import json
import math
import re
import subprocess
from pathlib import Path

from tqdm import tqdm


ANGLE_EPS = 1e-9
MOVE_RE = re.compile(r"^\s*(-?\d+)\s+(-?\d+)\s+(\S+)")


class ReplaySkip(Exception):
    pass


def load_replay(path: Path) -> dict:
    opener = gzip.open if path.suffix == ".gz" else open
    with opener(path, "rt", encoding="utf-8") as handle:
        return json.load(handle)


def extract_frames(payload: dict) -> list[dict]:
    if payload.get("code") == "UNAUTHORIZED":
        raise ReplaySkip("unauthorized replay payload")
    if "success" in payload:
      return payload["success"]["gameResult"]["frames"]
    if "frames" in payload:
        return payload["frames"]
    raise ReplaySkip("unsupported replay payload")


def split_view(frame: dict) -> list[str]:
    return frame["view"].splitlines()


def normalize_view(view_lines: list[str]) -> list[str]:
    if len(view_lines) > 8 and view_lines[1] == "CodersStrikeBack":
        return [view_lines[0]] + view_lines[5:]
    return view_lines


def parse_setup(frames: list[dict]) -> tuple[list[tuple[int, int]], list[dict[str, int | float | None]]]:
    setup = split_view(frames[0])
    if len(setup) < 4 or (len(setup) > 1 and setup[1] != "CodersStrikeBack"):
        raise ReplaySkip("not a Coders Strike Back replay")
    checkpoint_values = [int(value) for value in setup[3].split()]
    checkpoints = [
        (checkpoint_values[index], checkpoint_values[index + 1])
        for index in range(0, len(checkpoint_values), 2)
    ]

    current_state = parse_keyframe_state(normalize_view(split_view(frames[0])))
    return checkpoints, current_state


def parse_keyframe_state(view_lines: list[str]) -> list[dict[str, int | float | None]]:
    view_lines = normalize_view(view_lines)
    return [
        parse_pod_line(view_lines[1]),
        parse_pod_line(view_lines[3]),
        parse_pod_line(view_lines[5]),
        parse_pod_line(view_lines[7]),
    ]


def parse_pod_line(line: str) -> dict[str, int | float | None]:
    values = line.split()
    angle_rad = None if values[8] == "null" else float(values[8])
    return {
        "x": int(float(values[0])),
        "y": int(float(values[1])),
        "vx": int(float(values[2])),
        "vy": int(float(values[3])),
        "thrust": None if values[4] == "null" else int(values[4]),
        "target_x": None if values[6] == "null" else int(float(values[6])),
        "target_y": None if values[7] == "null" else int(float(values[7])),
        "angle_rad": -1.0 if angle_rad is None else angle_rad,
        "shield_used": int(values[9]),
        "next": int(values[10]),
    }


def pod_state_fields(pod: dict[str, int | float | None]) -> tuple[int, int, int, int, float, int]:
    return (
        int(pod["x"]),
        int(pod["y"]),
        int(pod["vx"]),
        int(pod["vy"]),
        float(pod["angle_rad"]),
        int(pod["next"]),
    )


def derive_move(
    current_pod: dict[str, int | float | None],
    next_pod: dict[str, int | float | None],
    current_boosted: int,
) -> str:
    target_x = next_pod["target_x"]
    target_y = next_pod["target_y"]
    if target_x is None or target_y is None:
        target_x = current_pod["x"]
        target_y = current_pod["y"]

    if int(next_pod["shield_used"]) != 0:
        token = "SHIELD"
    elif int(next_pod["thrust"] or 0) == 650 and current_boosted == 0:
        token = "BOOST"
    else:
        token = str(int(next_pod["thrust"] or 0))

    return f"{int(target_x)} {int(target_y)} {token}"


def parse_stdout_move(line: str) -> str | None:
    match = MOVE_RE.match(line)
    if match is None:
        return None

    thrust = match.group(3)
    if thrust not in {"BOOST", "SHIELD"}:
        try:
            thrust_value = int(thrust)
        except ValueError:
            return None
        if thrust_value < 0 or thrust_value > 200:
            return None

    return f"{match.group(1)} {match.group(2)} {thrust}"


def parse_stdout_moves(frame: dict) -> list[str | None]:
    moves: list[str | None] = [None, None]
    stdout = frame.get("stdout")
    if not stdout:
        return moves

    lines = [line for line in stdout.splitlines() if line.strip()]
    if len(lines) < 2:
        return moves

    parsed = [parse_stdout_move(line) for line in lines[:2]]
    if any(move is None for move in parsed):
        return moves

    return [parsed[0], parsed[1]]


def collect_turns(
    frames: list[dict],
) -> list[
    tuple[
        list[dict[str, int | float | None]],
        list[dict[str, int | float | None]],
        list[str | None],
    ]
]:
    turns = []
    current_state = parse_keyframe_state(split_view(frames[0]))
    pending_moves: list[str | None] = [None, None, None, None]

    for frame in frames[1:]:
        if frame.get("agentId") in {0, 1}:
            parsed = parse_stdout_moves(frame)
            offset = 0 if int(frame["agentId"]) == 0 else 2
            for index, move in enumerate(parsed):
                if move is not None:
                    pending_moves[offset + index] = move

        if not frame.get("keyframe"):
            continue

        next_state = parse_keyframe_state(split_view(frame))
        turns.append((current_state, next_state, pending_moves[:]))
        pending_moves = [None, None, None, None]
        current_state = next_state

    return turns


def angle_diff_rad(left: float, right: float) -> float:
    diff = (left - right) % (2.0 * math.pi)
    if diff > math.pi:
        diff -= 2.0 * math.pi
    return abs(diff)


def advance_boosted(moves: list[str], boosted: list[int]) -> list[int]:
    next_boosted = boosted[:]
    for pod_id, move in enumerate(moves):
        token = move.split(maxsplit=3)[2]
        if token == "BOOST" and next_boosted[pod_id] == 0:
            next_boosted[pod_id] = 1
    return next_boosted


def advance_shields(moves: list[str], shields: list[int]) -> list[int]:
    next_shields = shields[:]
    for pod_id, move in enumerate(moves):
        token = move.split(maxsplit=3)[2]
        if token == "SHIELD":
            next_shields[pod_id] = 3
        elif next_shields[pod_id] > 0:
            next_shields[pod_id] -= 1
    return next_shields


def resolve_moves(
    state: list[dict[str, int | float | None]],
    next_state: list[dict[str, int | float | None]],
    replay_moves: list[str | None],
    boosted: list[int],
) -> list[str]:
    moves: list[str] = []
    for pod_id in range(4):
        replay_move = replay_moves[pod_id]
        next_pod = next_state[pod_id]
        move = derive_move(state[pod_id], next_pod, boosted[pod_id])
        if replay_move is None:
            moves.append(move)
            continue

        replay_x, replay_y, replay_token = replay_move.split(maxsplit=2)
        derived_x, derived_y, derived_token = move.split(maxsplit=2)
        target_known = next_pod["target_x"] is not None and next_pod["target_y"] is not None

        target_x = derived_x
        target_y = derived_y
        if replay_token == "SHIELD" or target_known:
            target_x = replay_x
            target_y = replay_y

        token = derived_token
        if replay_token == "SHIELD":
            token = "SHIELD"
        elif replay_token == "BOOST" and derived_token == "BOOST":
            token = "BOOST"

        moves.append(f"{target_x} {target_y} {token}")
    return moves


def build_harness_input(
    checkpoints: list[tuple[int, int]],
    turns: list[
        tuple[
            list[dict[str, int | float | None]],
            list[dict[str, int | float | None]],
            list[str | None],
        ]
    ],
) -> str:
    progress = [1, 1, 1, 1]
    boosted = [0, 0, 0, 0]
    shields = [0, 0, 0, 0]
    parts = [str(len(checkpoints))]
    parts.extend(f"{x} {y}" for x, y in checkpoints)
    parts.append(str(len(turns)))

    for state, next_state, replay_moves in turns:
        moves = resolve_moves(state, next_state, replay_moves, boosted)
        for pod_id, pod in enumerate(state):
            parts.append(
                f"{int(pod['x'])} {int(pod['y'])} {int(pod['vx'])} {int(pod['vy'])} "
                f"{float(pod['angle_rad'])} {int(pod['next'])} {shields[pod_id]} {boosted[pod_id]} {progress[pod_id]}"
            )
        parts.extend(moves)
        boosted = advance_boosted(moves, boosted)
        shields = advance_shields(moves, shields)
        for pod_id, pod in enumerate(next_state):
            if int(pod["next"]) != int(state[pod_id]["next"]):
                progress[pod_id] += 1

    return "\n".join(parts) + "\n"


def parse_harness_output(output: str) -> list[list[float]]:
    lines = [line.strip() for line in output.splitlines() if line.strip()]
    return [[float(value) for value in line.split()] for line in lines]


def validate_replay(binary: Path, replay: Path) -> tuple[str, str]:
    try:
        payload = load_replay(replay)
        frames = extract_frames(payload)
        checkpoints, _ = parse_setup(frames)
        turns = collect_turns(frames)
    except ReplaySkip as exc:
        return "skip", str(exc)
    except Exception as exc:  # noqa: BLE001
        return "fail", f"replay parse error: {exc}"

    if not turns:
        return "skip", "no complete turns found"

    harness_input = build_harness_input(checkpoints, turns)
    command = [str(binary)]

    result = subprocess.run(
        command,
        input=harness_input,
        text=True,
        capture_output=True,
        check=False,
    )
    if result.returncode != 0:
        return "fail", f"binary exited with {result.returncode}: {result.stderr.strip()}"

    rows = parse_harness_output(result.stdout)
    expected_rows = len(turns) * 4
    if len(rows) != expected_rows:
        return "fail", f"expected {expected_rows} output rows, got {len(rows)}"

    row_index = 0

    boosted = [0, 0, 0, 0]
    shields = [0, 0, 0, 0]

    for turn_index, (state, next_state, replay_moves) in enumerate(turns):
        moves = resolve_moves(state, next_state, replay_moves, boosted)
        boosted = advance_boosted(moves, boosted)
        shields = advance_shields(moves, shields)
        for pod_id in range(4):
            row = rows[row_index]
            row_index += 1

            expected = next_state[pod_id]
            expected_state = pod_state_fields(expected)
            if int(row[0]) != expected_state[0] or int(row[1]) != expected_state[1]:
                return "fail", f"turn {turn_index} pod {pod_id}: position mismatch"
            if int(row[2]) != expected_state[2] or int(row[3]) != expected_state[3]:
                return "fail", f"turn {turn_index} pod {pod_id}: velocity mismatch"
            if angle_diff_rad(row[4], expected_state[4]) > ANGLE_EPS:
                return "fail", f"turn {turn_index} pod {pod_id}: angle mismatch"
            if int(row[5]) != expected_state[5]:
                if turn_index != len(turns) - 1:
                    return "fail", f"turn {turn_index} pod {pod_id}: next checkpoint mismatch"
            if int(row[6]) != shields[pod_id]:
                return "fail", f"turn {turn_index} pod {pod_id}: shield mismatch"
            if int(row[7]) != boosted[pod_id]:
                return "fail", f"turn {turn_index} pod {pod_id}: boost mismatch"

    return "pass", f"{len(turns)} turns validated"


def expand_replays(paths: list[str]) -> list[Path]:
    results: list[Path] = []
    for raw in paths:
        path = Path(raw)
        if path.is_dir():
            results.extend(sorted(path.glob("*.json")))
            results.extend(sorted(path.glob("*.json.gz")))
        else:
            results.append(path)
    return results


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate replay files against the simulator harness.")
    parser.add_argument("--binary", required=True, type=Path, help="Compiled simulator harness binary.")
    parser.add_argument("--replays", nargs="+", required=True, help="Replay files or directories.")
    args = parser.parse_args()

    replays = expand_replays(args.replays)
    if not replays:
        raise SystemExit("No replay files found.")

    passed = 0
    skipped = 0
    failed = 0
    for replay in tqdm(replays, desc="Replays"):
        status, message = validate_replay(args.binary, replay)
        if status == "pass":
            passed += 1
        elif status == "skip":
            skipped += 1
            print(f"SKIP {replay}: {message}")
        else:
            failed += 1
            print(f"FAIL {replay}: {message}")

    comparable = len(replays) - skipped
    print(f"\n{passed}/{comparable} comparable replays passed ({skipped} skipped)")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
