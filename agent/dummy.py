import torch
import math

from state import State

class DummyAgent:

  @torch.no_grad()
  def act(self, state: State) -> torch.Tensor
    B = state.raw.shape[0]
    moves = torch.zeros((B, 2, 4), dtype=torch.float32, device=state.raw.device)
    for p in (0, 1):
      cp_xy = get_checkpoint_xy(state, state.next_cp[:, p])
      target_angle = torch.atan2(
        cp_xy[:, 1] - state.y[:, p], cp_xy[:, 0] - state.x[:, p]
      )
      diff = (target_angle - state.angle[:, p] + math.pi) % (
        2.0 * math.pi
      ) - math.pi
      moves[:, p, 0] = diff.clamp(-MAX_ROTATION, +MAX_ROTATION)
      moves[:, p, 1] = 100.0
    return moves

  def decode_action(self, action: torch.Tensor):
    return action

  def encode_state(self, state: State):
    return state
