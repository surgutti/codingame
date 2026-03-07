#!/usr/bin/env python3

from __future__ import annotations

import gzip
import json
import sqlite3
import time
from datetime import datetime
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen
try:
    from tqdm import tqdm
except ModuleNotFoundError:
    def tqdm(iterable, **_: object):
        return iterable


BASE_DIR = Path(__file__).resolve().parent
DATA_DIR = BASE_DIR / "replays"
DB_PATH = BASE_DIR / "replays.db"
API_BASE = "https://www.codingame.com/services"
CONTEST = "coders-strike-back"
MAX_PLAYERS = 150
REQUEST_DELAY = 0.05
UNAVAILABLE_REPLAYS = {
    "385811897": "not_csb",
    "859958644": "invalid_payload",
    "864047603": "unauthorized",
    "864047604": "unauthorized",
}


def open_db() -> sqlite3.Connection:
    connection = sqlite3.connect(DB_PATH)
    connection.execute(
        """
        CREATE TABLE IF NOT EXISTS replays (
          replay_id TEXT PRIMARY KEY,
          competition_id TEXT,
          downloaded_at TEXT,
          file_path TEXT,
          status TEXT
        )
        """
    )
    return connection


def post_json(path: str, payload: list) -> dict | list:
    body = json.dumps(payload).encode("utf-8")
    request = Request(
        f"{API_BASE}/{path}",
        data=body,
        headers={
            "Content-Type": "application/json",
            "Accept": "application/json",
        },
        method="POST",
    )
    with urlopen(request, timeout=30) as response:
        return json.load(response)


def get_leaderboard() -> list[dict]:
    payload = [
        CONTEST,
        "ea0ce7b64a1fb206d6fcfcd0841eaaab5264483",
        "global",
        {"active": "true", "column": "COUNTRY", "filter": "ALL"},
    ]
    response = post_json("Leaderboards/getFilteredPuzzleLeaderboard", payload)
    return response["users"][:MAX_PLAYERS]


def get_battles(agent_id: str) -> list[dict]:
    return post_json("gamesPlayersRanking/findLastBattlesByAgentId", [str(agent_id), None])


def get_replay(replay_id: str) -> dict:
    return post_json("gameResult/findByGameId", [replay_id, None])


def replay_known(connection: sqlite3.Connection, replay_id: str) -> bool:
    if replay_id in UNAVAILABLE_REPLAYS:
        return True

    row = connection.execute(
        "SELECT 1 FROM replays WHERE replay_id = ?",
        (replay_id,),
    ).fetchone()
    return row is not None


def record_replay(
    connection: sqlite3.Connection,
    replay_id: str,
    status: str,
    file_path: str | None = None,
) -> None:
    connection.execute(
        """
        INSERT OR REPLACE INTO replays
        (replay_id, competition_id, downloaded_at, file_path, status)
        VALUES (?, ?, ?, ?, ?)
        """,
        (
            replay_id,
            CONTEST,
            datetime.now().isoformat(),
            file_path,
            status,
        ),
    )
    connection.commit()


def save_replay(
    connection: sqlite3.Connection,
    replay_id: str,
    payload: dict,
) -> None:
    file_path = DATA_DIR / f"{replay_id}.json.gz"
    with gzip.open(file_path, "wt", encoding="utf-8") as handle:
        json.dump(payload, handle, ensure_ascii=False)
    record_replay(connection, replay_id, "ok", str(file_path))


def main() -> int:
    DATA_DIR.mkdir(exist_ok=True)
    connection = open_db()

    try:
        top_players = get_leaderboard()
        top_agents = [player["agentId"] for player in top_players]
        print("Top agent ids:", top_agents)

        for agent_id in tqdm(top_agents, desc="Agents"):
            battles = get_battles(agent_id)
            for battle in tqdm(battles, desc=f"Battles {agent_id}", leave=False):
                replay_id = str(battle["gameId"])
                if replay_known(connection, replay_id):
                    continue

                payload = get_replay(replay_id)
                save_replay(connection, replay_id, payload)
                time.sleep(REQUEST_DELAY)
    except (HTTPError, URLError) as exc:
        raise SystemExit(f"download failed: {exc}") from exc

    connection.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
