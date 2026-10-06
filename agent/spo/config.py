import torch
from dataclasses import dataclass

@dataclass
class SPOConfig:
  name: str = "simple-mlp"
  device: str = "gpu" if torch.cuda.is_available() else "cpu"
  max_bots: int = 9
  seed: int = 42
  state_dim: int = 80

  total_episodes: int = 1_000_000
  episode_steps: int = 256 # 512
  num_envs: int = 4 # 512
  minibatch_size: int = 128
  episodes_per_gen: int = 200

  learning_rate: float = 3e-5
  gamma: float = 0.995
  gae_lambda: float = 0.95
  grad_clip: float = 0.5

  update_epochs: int = 4

  norm_adv: bool = True
  clip_eps_low: float = 0.2
  clip_eps_high: float = 0.28
  clip_vloss: bool = False
  ent_coef: float = 0.003
  vf_coef: float = 0.5

  scale_vf: bool = True
  adv_tail_c: float = 4.0
  dual_clip_c: float = 3.0
  spo_ratio_cap: float = 3.0
  target_kl: float = 100 # 0.04
  refresh_gae_every_epoch: bool = False
  use_huber_vf: bool = True
  scale_critic_targets: bool = True
