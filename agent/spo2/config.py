import torch
from dataclasses import dataclass

@dataclass
class SPO2Config:
  name: str = "swiglu-gen2"
  device: str = "cuda" if torch.cuda.is_available() else "cpu"
  seed: int = 42
  state_dim: int = 102

  learning_rate: float = 1e-4
  gamma: float = 0.995
  gae_lambda: float = 0.97
  grad_clip: float = 0.67

  minibatch_size: int = 65536
  update_epochs: int = 4

  norm_adv: bool = True
  clip_eps_low: float = 0.2
  clip_eps_high: float = 0.28
  clip_vloss: bool = False
  ent_coef: float = 0.002
  vf_coef: float = 0.5

  scale_vf: bool = True
  adv_tail_c: float = 4.0
  dual_clip_c: float = 3.0
  spo_ratio_cap: float = 3.0
  target_kl: float = 100 # 0.04
  refresh_gae_every_epoch: bool = False
  use_huber_vf: bool = True
  scale_critic_targets: bool = True
