import torch
from dataclasses import dataclass

@dataclass
class Config:

  warmup_steps: int = 512
  device: str = "cuda" if torch.cuda.is_available() else "cpu"
  seed: int = 42
  state_dim: int = 80

  total_episodes: int = 1_000_000
  episode_steps: int = 1024
  num_envs: int = 256
  episodes_per_gen: int = 1000

  champion_threshold: float = 0.95
  champion_steps: int = 100
  max_bots: int = 20
