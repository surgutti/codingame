from dataclasses import dataclass

@dataclass
class PPOConfig:
  name: str = "init"
  device: str = "cpu"
  seed: int = 42

  total_episodes: int = 1_000_000
  episode_steps: int = 256
  num_envs: int = 64
  minibatch_size: int = 256

  learning_rate: float = 2e-4
  gamma: float = 0.99
  gae_lambda: float = 0.95

  update_epochs: int = 16

  norm_adv: bool = True
  clip_coef: float = 0.2
  clip_vloss: bool = True
  ent_coef: float = 0.01
  vf_coef: float = 0.5

