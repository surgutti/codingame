from dataclasses import dataclass

@dataclass
class PPOConfig:
  name: str = "init"
  device: str = "cuda"
  max_bots: int = 9
  seed: int = 42

  total_episodes: int = 1_000_000
  episode_steps: int = 512
  num_envs: int = 512
  minibatch_size: int = 4096
  episodes_per_gen: int = 50

  learning_rate: float = 1e-4
  gamma: float = 0.995
  gae_lambda: float = 0.95
  grad_clip: float = 0.5

  update_epochs: int = 4

  norm_adv: bool = True
  clip_eps: float = 0.2
  clip_vloss: bool = False
  ent_coef: float = 0.003
  vf_coef: float = 0.5

