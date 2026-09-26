import torch
import math

from state import State, get_checkpoint_xy, MAX_ROT

class DummyAgent:

  @torch.no_grad()
  def act(self, state: State) -> torch.Tensor:
    moves = torch.zeros((*state.raw.shape[:-1], 2, 4), dtype=torch.float32, device=state.raw.device)
    for p in (0, 1):
      cp_xy = get_checkpoint_xy(state, state.next_cp[..., p])
      target_angle = torch.atan2(
        cp_xy[..., 1] - state.y[..., p], cp_xy[..., 0] - state.x[..., p]
      )
      diff = (target_angle - state.angle[..., p] + math.pi) % (
        2.0 * math.pi
      ) - math.pi

      moves[..., p, 0] = diff.clamp(-MAX_ROT, +MAX_ROT)
      moves[..., p, 1] = 100.0
      #moves[..., p, 2] = 0.0
      #moves[..., p, 3] = 0.0
    return moves.view(*state.raw.shape[:-1], 8)

  def decode_action(self, action: torch.Tensor):
    return action

  def encode_state(self, state: State):
    return state
