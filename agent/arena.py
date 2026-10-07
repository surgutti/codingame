import os
import json
import torch
import shutil
import random

import torch.nn.functional as F
from torch.utils.tensorboard import SummaryWriter
from dataclasses import dataclass, field, asdict, is_dataclass

from env import VecEnv
from agent import Agent, AGENT_REGISTRY
from entry import BotEntry, JSONFix
from config import ArenaConfig
from utils import calculate_nash_weights

class Arena:
  def __init__(
    self,
    config: ArenaConfig,
    env: VecEnv,
    save_dir: str = "checkpoints"
  ):
    self.config = config
    self.save_dir = save_dir
    self.bots_log_root = f"runs/{config.name}/bots"
    self.state_path = os.path.join(save_dir, "arena_state.json")
    self.env = env
    os.makedirs(save_dir, exist_ok=True)

    self.pool: list[BotEntry] = []

    self.wins = torch.zeros((0, 0), dtype=torch.int32)
    self.draws = torch.zeros((0, 0), dtype=torch.int32)
    self.losses = torch.zeros((0, 0), dtype=torch.int32)
  
  def save_state(
    self,
    next_gen: int,
    global_ep: int
  ):
    state_data = {
      "next_gen": next_gen,
      "global_ep": global_ep,
      "pool": [b.to_dict() for b in self.pool]
    }
    tmp_state = self.state_path + ".tmp"
    with open(tmp_state, "w", encoding="utf-8") as f:
      json.dump(state_data, f, indent=2, cls=JSONFix)
    os.replace(tmp_state, self.state_path)
  
  def load_state(self) -> tuple[int, int]:
    if not (os.path.exists(self.state_path) and os.path.exists(self.learner_path)):
      return 0, 0

    with open(self.state_path, "r", encoding="utf-8") as f:
      data = json.load(f)

    next_gen = int(data["next_gen"])
    global_ep = int(data["global_ep"])

    for b in self.pool:
      if b.writer is not None:
        b.writer.close()

    loaded_pool: list[BotEntry] = []
    for item in data.get("pool", []):
      path = item.get("path")
      if path is not None and not os.path.exists(path):
        continue

      agent_kwargs = item.get("agent_kwargs", {})
      if "config" in agent_kwargs and isinstance(agent_kwargs["config"], dict):
        agent_kwargs["config"] = SPOConfig(**agent_kwargs["config"])

      loaded_pool.append(
        BotEntry(
          name=item["name"],
          path=path,
          log_dir=item.get("log_dir"),
          pool_idx=item.get("pool_idx"),
          agent_type=item.get("agent_type"),
          agent_kwargs=agent_kwargs,
          purge_step=global_ep,
        )
      )

    if loaded_pool:
      self.pool = loaded_pool

    print(
      f"[Arena] Resumed from gen={next_gen}, global_ep={global_ep}, "
      f"pool_size={len(self.pool)}"
    )
    return next_gen, global_ep

  def close(self):
    for b in self.pool:
      if b.writer is not None:
        b.writer.close()

  def load_bot(self, entry: BotEntry):
    if entry._instance is not None:
      return entry._instance

    agent_cls = AGENT_REGISTRY.get(entry.agent_type)
    if agent_cls is None:
      raise ValueError(f"Unknown type: {entry.agent_type}")

    agent = agent_cls(**entry.agent_kwargs)

    if entry.path is not None and os.path.exists(entry.path):
      ckpt = torch.load(entry.path, map_location=self.config.device, weights_only=True)
      if isinstance(agent, torch.nn.Module):
        agent.load_state_dict(ckpt)
      
    entry._instance = agent
    return agent 

  @torch.no_grad()
  def play_match(self, agent0: Agent, agent1: Agent, steps: int = 512):
    s0, s1 = self.env.reset()
    wins = torch.zeros((), device=self.config.device)
    losses = torch.zeros((), device=self.config.device)
    draws = torch.zeros((), device=self.config.device)
    for _ in range(steps):
      a0 = agent0.act(s0)
      a1 = agent1.act(s1)
      s0, s1, _, done = self.env.step(torch.cat([a0, a1], dim=-1))
      wins += (done == 1.0).sum()
      losses += (done == 2.0).sum()
      draws += (done == 3.0).sum()
    
    return wins.item(), losses.item(), draws.item()

  def record_match(
    self,
    entry0: BotEntry, entry1: BotEntry,
    w: int, d: int, l: int
  ):
    idx0, idx1 = entry0.pool_idx, entry1.pool_idx

    self.wins[idx0][idx1] += w 
    self.draws[idx0][idx1] += d
    self.losses[idx0][idx1] += l

    self.wins[idx1][idx0] += l
    self.draws[idx1][idx0] += d
    self.losses[idx1][idx0] += w

  def register(
    self, 
    name: str,
    agent: Agent,
    agent_type: str,
    agent_kwargs: dict | None = None,
  ) -> BotEntry:
    path = os.path.join(self.save_dir, f"{name}.pt")
    torch.save(agent.state_dict(), path)

    bot_idx = len(self.pool)
    bot_entry = BotEntry(
      name=name, 
      path=path, 
      log_dir=os.path.join(self.bots_log_root, name),
      pool_idx=bot_idx,
      agent_type=agent_type,
      agent_kwargs=agent_kwargs or {},
    )
    bot_entry._instance = agent
    
    self.pool.append(bot_entry)
    self.wins = F.pad(self.wins, (0, 1, 0, 1), mode='constant', value=0)
    self.draws = F.pad(self.draws, (0, 1, 0, 1), mode='constant', value=0)
    self.losses = F.pad(self.losses, (0, 1, 0, 1), mode='constant', value=0)
    
    for opp_entry in self.pool:
      opp_agent = self.load_bot(opp_entry)
      w, d, l = self.play_match(agent, opp_agent, steps=self.config.warmup_steps)
      self.record_match(bot_entry, opp_entry, w, d, l)

    if len(self.pool) > config.max_bots:
      weights = self.nash_weights()
      weak_bot = -1
      for i, entry in enumerate(self.pool):
        if entry.type != "baseline" and \
           (weak_bot == -1 or weights[weak_bot] > weighs[i]):
          weak_bot = i

      # it is not used anywhere and there are too many agents in the leaderboard
      if weak_bot != -1 and weights[weak_bot] < 1e-10:
        self.remove_bot(weak_bot)

    return bot_entry

  # wins with the nash equilibrium with high probability
  def is_champion(self, agent: Agent):
    weights = self.nash_weights()
   
    total_wr = 0 
    for i, opp_entry in enumerate(self.pool):
      opp_agent = self.load_bot(opp_entry)
      w, d, l = self.play_match(agent, opp_agent, steps=self.config.champion_steps)
      wr = (w + 0.5 * d) / max(1, w + d + l)

      total_wr += weights[i] * wr

    return total_wr >= config.champion_threshold, total_wr

  def sample_opponent(self, weights):
    entry = random.choices(self.pool, weights=weights, k=1)[0]
    return entry, self.load_bot(entry)

  def nash_weights(self):
    return calculate_nash_weights(
      self.wins.numpy(),
      self.draws.numpy(),
      self.losses.numpy(),
    )

  def remove_bot(self, bot_idx):
    mask = torch.ones(len(self.pool), dtype=torch.bool)
    mask[i] = False

    self.wins = self.wins[mask, :][:, mask]
    self.draws = self.draws[mask, :][:, mask]
    self.losses = self.losses[mask, :][:, mask]
    
    self.pool.pop(bot_idx

  def stabilize(self, num_matches: int = 20):
    if len(self.pool) < 2:
      return

    for _ in range(num_matches):
      entry0, entry1 = random.sample(self.pool, k=2)

      agent0 = self.load_bot(entry_a)
      agent1 = self.load_bot(entry_b)

      w, d, l = self.play_match(agent0, agent1)
      self.record_match(entry0, entry1, w, d, l)
