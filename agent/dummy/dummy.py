import torch
import torch.nn as nn
import math

from state import State, get_checkpoint_xy, MAX_ROT

class DummyAgent(nn.Module):
  def __init__(self):
    super().__init__()
  @torch.no_grad()
  @torch.compile(fullgraph=True)
  def act(self, state: State) -> torch.Tensor:
    B, E, _ = state.raw.shape
    cps = state.checkpoints
    idx = (state.next_cp[:, :, :2] % state.num_cps.unsqueeze(-1)).unsqueeze(-1).expand(B, E, 2, 2)
    cp_xy = torch.gather(cps, dim=2, index=idx)
    target_angle = torch.atan2(cp_xy[:, :, :, 1] - state.y[:, :, :2], cp_xy[:, :, :, 0] - state.x[:, :, :2])
    diff = (target_angle - state.angle[:, :, :2] + math.pi) % (2.0 * math.pi) - math.pi
    moves = torch.zeros((B, E, 2, 4), dtype=torch.float32, device=state.raw.device)
    moves[:, :, :, 0] = diff.clamp(-MAX_ROT, +MAX_ROT)
    moves[:, :, :, 1] = 100.0
    return moves.view(B, E, 8)
  def observe(self, reward, next_state, done):
    return
  def update(self):
    return {}
