from dataclasses import dataclass

@dataclass
class SPOConfig:
  name: str = "swiglu"
  device: str = "cpu"
  max_bots: int = 9
  seed: int = 42
  state_dim: int = 48

  total_episodes: int = 1_000_000
  episode_steps: int = 64
  num_envs: int = 8
  minibatch_size: int = 256
  episodes_per_gen: int = 10

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

