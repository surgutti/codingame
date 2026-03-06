#!/usr/bin/env python3

import requests
import json
import gzip
import sqlite3
import time
from datetime import datetime
from pathlib import Path
from tqdm import tqdm

DATA_DIR = Path("replays")
DATA_DIR.mkdir(exist_ok=True)

API_BASE = "https://www.codingame.com/services"
CONTEST = "coders-strike-back" #"mad-pod-racing"
REPLAYS_DB = "replays.db"

conn = sqlite3.connect(REPLAYS_DB)
conn.execute("""
CREATE TABLE IF NOT EXISTS replays (
  replay_id TEXT PRIMARY KEY,
  competition_id TEXT,
  downloaded_at TEXT,
  file_path TEXT,
  status TEXT
)          
""")

def getFilteredPuzzleLeaderboard(contest_name: str):
  ranks = requests.post(
    f"{API_BASE}/Leaderboards/getFilteredPuzzleLeaderboard",
    json = [
      contest_name,
      "ea0ce7b64a1fb206d6fcfcd0841eaaab5264483",
      "global",
      {
        "active": "true",
        "column": "COUNTRY",
        "filter": "ALL"
      }
    ]
  ).json()

  return ranks['users']

def findLastBattlesByAgentId(agent_id):
  battles = requests.post(
    f"{API_BASE}/gamesPlayersRanking/findLastBattlesByAgentId",
    json = [
      str(agent_id),
      None
    ]
  ).json()

  return battles

def findByGameId(game_id: str):
  replay = requests.post(
    f"{API_BASE}/gameResult/findByGameId",
    json = [
      game_id,
      None
    ]
  ).json()

  return replay

def replayExists(replay_id: str) -> bool:
  row = conn.execute(
    "SELECT 1 FROM replays WHERE replay_id = ?",
    (replay_id,)
  ).fetchone()
  return row is not None

def saveReplay(replay_id: str, competition_id: str, payload: dict) -> None:
  file_path = DATA_DIR / f"{replay_id}.json.gz"

  with gzip.open(file_path, "wt", encoding="utf-8") as f:
    json.dump(payload, f, ensure_ascii=False)
  
  conn.execute("""
    INSERT OR REPLACE INTO replays
    (replay_id, competition_id, downloaded_at, file_path, status)
    VALUES (?, ?, ?, ?, ?)          
  """, (
    replay_id,
    competition_id,
    datetime.now().isoformat(),
    str(file_path),
    "ok"
  ))
  conn.commit()

def main():

  top_players = getFilteredPuzzleLeaderboard(CONTEST)[:10]
  top_agents = [player['agentId'] for player in top_players]
  print("Top agents ids: ", top_agents)

  for agent in tqdm(top_agents, desc="Agents"):
    battles = findLastBattlesByAgentId(agent)

    for battle in tqdm(battles, desc=f"Battles {agent}", leave=False):
      replay_id = str(battle['gameId'])

      if replayExists(replay_id):
        continue

      replay = findByGameId(replay_id)

      saveReplay(replay_id, CONTEST, replay)

      time.sleep(0.05)

if __name__ == "__main__":
  raise SystemExit(main())
