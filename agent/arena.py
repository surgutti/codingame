import os
import torch
import shutil
import random

from torch.utils.tensorboard import SummaryWriter
from dataclasses import dataclass, field

from env import VecEnv
from ppo import PPOAgent
from dummy import DummyAgent

@dataclass
class BotEntry:
  name: str
  path: str | None
  log_dir: str | None = None
  elo: float = 1000.0
  wins: float = 0.0
  losses: float = 0.0
  draws: float = 0.0
  recent_win_rate:  float = 0.5
  writer: SummaryWriter | None = field(init=False, default=None, repr=False)

  def __post_init__(self):
    if self.log_dir is not None:
      self.writer = SummaryWriter(log_dir=self.log_dir)

  @property
  def games(self) -> int:
    return int(self.wins + self.losses + self.draws)

  def record_result(self, w: float, l: float, d: float, ema_alpha: float = 0.2):
    total = w + l + d
    if total <= 0:
      return

    self.wins += w
    self.losses += l
    self.draws += d

    batch_wr = (w + 0.5 * d) / total
    self.recent_win_rate = (1.0 - ema_alpha) * self.recent_win_rate + ema_alpha * batch_wr

  def log_step(self, global_ep: int, rank: int):
    if self.writer is None:
      return
    total = max(self.games, 1)
    self.writer.add_scalar("bot/elo", self.elo, global_ep)
    self.writer.add_scalar("bot/rank", rank, global_ep)
    self.writer.add_scalar("bot/win_rate_cum", (self.wins + 0.5 * self.draws) / total, global_ep)
    self.writer.add_scalar("bot/win_rate_recent", self.recent_win_rate, global_ep)
    self.writer.add_scalar("bot/games_played", self.games, global_ep)

  def cleanup(self):
    if self.writer is not None:
      self.writer.close()
    if self.path and os.path.exists(self.path):
      os.remove(self.path)
    if self.log_dir and os.path.exists(self.log_dir):
      shutil.rmtree(self.log_dir, ignore_errors=True)

class Arena:
  def __init__(
    self,
    config,
    state_dim: int,
    max_bots: int = 20,
    save_dir: str = "checkpoints"
  ):
    self.config = config
    self.state_dim = state_dim
    self.max_bots = max_bots
    self.save_dir = save_dir
    self.bots_log_root = f"runs/ppo_{config.name}/bots"
    os.makedirs(save_dir, exist_ok=True)

    self.pool: list[BotEntry] = [
      BotEntry(
        name="dummy",
        path=None,
        log_dir=os.path.join(self.bots_log_root, "dummy"),
        elo=1000.0
      )
    ]

    self._opponent_model = PPOAgent(config, state_dim).to(config.device)
    self._opponent_model.eval()

    self._dummy = DummyAgent()

  def load_bot(self, entry: BotEntry):
    if entry.path is None:
      return self._dummy

    ckpt = torch.load(entry.path, map_location=self.config.device, weights_only=True)
    self._opponent_model.load_state_dict(ckpt)
    self._opponent_model.eval()

    return self._opponent_model

  def sample_opponent(self, learner_elo: float):
    weights = [1.0 / (1.0 + abs(b.elo - learner_elo) / 200.0) for b in self.pool]
    entry = random.choices(self.pool, weights=weights, k=1)[0]
    return entry, self.load_bot(entry)

  @staticmethod
  def update_elo(
    a: BotEntry, 
    b: BotEntry, 
    wins: float, 
    losses: float, 
    draws: float, 
    k: float = 32.0
  ):
    total = wins + losses + draws
    if total == 0:
      return
    exp_a = 1.0 / (1.0 + 10.0 ** ((b.elo - a.elo) / 400.0))
    score_a = (wins + 0.5 * draws) / total
    delta = k * (score_a - exp_a)
    if a.path is not None:
      a.elo += delta
    if b.path is not None:
      b.elo -= delta

    a.record_result(wins, losses, draws)
    b.record_result(losses, wins, draws)

  @torch.no_grad()
  def play_match(self, envs: VecEnv, agent_a, agent_b, steps: int = 250):
    s0, s1 = envs.reset()
    wins = torch.zeros((), device=self.config.device)
    losses = torch.zeros((), device=self.config.device)
    draws = torch.zeros((), device=self.config.device)
    for _ in range(steps):
      a0 = agent_a.decode_action(agent_a.act(agent_a.encode_state(s0)))
      a1 = agent_b.decode_action(agent_b.act(agent_b.encode_state(s1)))
      s0, s1, _, done = envs.step(torch.cat([a0, a1], dim=-1))
      wins += (done == 1.0).sum()
      losses += (done == 2.0).sum()
      draws += (done == 3.0).sum()

    return wins.item(), losses.item(), draws.item()

  def register_and_prune(
    self, 
    gen: int, 
    learner: PPOAgent, 
    envs: VecEnv, 
    global_ep: int, 
    start_elo: float
  ) -> BotEntry:
    name = f"gen_{gen:04d}"
    path = os.path.join(self.save_dir, f"{name}.pt")
    torch.save(learner.state_dict(), path)
    new_bot = BotEntry(
      name=name, 
      path=path, 
      log_dir=os.path.join(self.bots_log_root, name),
      elo=start_elo,
    )

    opponents = random.sample(self.pool, k=min(3, len(self.pool)))
    for opp_entry in opponents:
      opp_agent = self.load_bot(opp_entry)
      w, l, d = self.play_match(envs, learner, opp_agent)
      self.update_elo(new_bot, opp_entry, w, l, d)

    self.pool.append(new_bot)

    neural_bots = sorted(
      [b for b in self.pool if b.path is not None], 
      key=lambda x: x.elo, 
      reverse=True
    )
    while len(neural_bots) > self.max_bots:
      dropped = neural_bots.pop()
      dropped.cleanup()

    self.pool = [b for b in self.pool if b.path is None] + neural_bots

    ranked_all = sorted(self.pool, key=lambda x: x.elo, reverse=True)
    for rank, bot in enumerate(ranked_all, start=1):
      bot.log_step(global_ep, rank)
    return new_bot


