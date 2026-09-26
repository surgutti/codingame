from dataclasses import dataclass
import torch

MAX_ROT = 0.3141592653589793

@dataclass
class State:
  raw: torch.Tensor # [B, 48]

  @property
  def pods(self) -> torch.Tensor:
    return self.raw[:, :32].view(-1, 4, 8) # [B, 4 pods, 8]

  @property
  def x(self) -> torch.Tensor:
    return self.pods[:, :, 0] # [B, 4]

  @property
  def y(self) -> torch.Tensor:
    return self.pods[:, :, 1] # [B, 4]

  @property
  def vx(self) -> torch:Tensor:
    return self.pods[:, :, 2] # [B, 4]

  @property
  def vy(self) -> torch.Tensor:
    return self.pods[:, :, 3] # [B, 4]

  @property
  def angle(self) -> torch.Tensor:
    return self.pods[:, :, 4] # [B, 4]

  @property
  def next_cp(self) -> torch.Tensor:
    return self.pods[:, :, 5].long() # [B, 4]

  @property
  def shield(self) -> torch.Tensor:
    return self.pods[:, :, 6] # [B, 4]

  @property
  def boosted(self) -> torch.Tensor:
    return self.pods[:, :, 7] # [B, 4]

  @property
  def checkpoints(self) -> torch.Tensor:
    return self.raw[:, 32:44].view(-1, 6, 2) # [B, 6 cps, 2]

  @property
  def num_cps(self) -> torch.Tensor:
    return self.raw[:, 44].long() # [B]

  def flip_teams(self) -> "State":
    flipped = self.raw.clone()
    flipped[:, 0:16] = self.raw[:, 16:32]
    flipped[:, 16:32] = self.raw[:, 0:16]
    flipped[:, 46] = self.raw[:, 47]
    flipped[:, 47] = self.raw[:, 46]
    return State(flipped)

def get_checkpoint_xy(state: State, cp_idx: torch.Tensor) -> torch.Tensor:
  B = state.raw.shape[0]
  batch_idx = torch.arange(B, device=state.raw.device).view(
    B, *([1] * (cp_idx.dim() - 1))
  )
  mod_idx = cp_idx % state.num_cps.view(B, *([1] * (cp_idx.dim() - 1)))
  return state.checkpoints[batch_idx, mod_idx]
