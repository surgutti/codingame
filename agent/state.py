from dataclasses import dataclass
import torch

MAX_ROT = 0.3141592653589793

@dataclass
class State:
  raw: torch.Tensor # [B, E, 48]

  @property
  def pods(self) -> torch.Tensor:
    return self.raw[..., :32].view(*self.raw.shape[:-1], 4, 8)

  @property
  def x(self) -> torch.Tensor:
    return self.pods[..., 0] # [B, E, 4]

  @property
  def y(self) -> torch.Tensor:
    return self.pods[..., 1] # [B, E, 4]

  @property
  def vx(self) -> torch.Tensor:
    return self.pods[..., 2] # [B, E, 4]

  @property
  def vy(self) -> torch.Tensor:
    return self.pods[..., 3] # [B, E, 4]

  @property
  def angle(self) -> torch.Tensor:
    return self.pods[..., 4] # [B, E, 4]

  @property
  def next_cp(self) -> torch.Tensor:
    return self.pods[..., 5].long() # [B, E, 4]

  @property
  def shield(self) -> torch.Tensor:
    return self.pods[..., 6] # [B, E, 4]

  @property
  def boosted(self) -> torch.Tensor:
    return self.pods[..., 7] # [B, E, 4]
  
  @property
  def timeouts(self) -> torch.Tensor:
    return self.raw[..., 46:48] # [B, E, 2]

  @property
  def checkpoints(self) -> torch.Tensor:
    return self.raw[..., 32:44].view(*self.raw.shape[:-1], 6, 2) # [B, E, 6 cps, 2]

  @property
  def num_cps(self) -> torch.Tensor:
    return self.raw[..., 44].long() # [B, E]

  def flip_teams(self) -> "State":
    flipped = self.raw.clone()
    flipped[..., 0:16] = self.raw[..., 16:32]
    flipped[..., 16:32] = self.raw[..., 0:16]
    flipped[..., 46] = self.raw[..., 47]
    flipped[..., 47] = self.raw[..., 46]
    return State(flipped)

def get_checkpoint_xy(state: State, cp_idx: torch.Tensor) -> torch.Tensor:
  mod_idx = cp_idx % state.num_cps
  gather_idx = mod_idx.unsqueeze(-1).unsqueeze(-1).expand(*mod_idx.shape, 1, 2)
  result = torch.gather(state.checkpoints, dim=-2, index=gather_idx)
  return result.squeeze(-2)
