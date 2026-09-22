import torch import torch.nn as nn
from torch.distributions.categorical import Categorical
from state import State, get_checkpoint_xy

def compute_progress(state: State) -> torch.Tensor:
  cp_xy = get_checkpoint_xy(state, state.next_cp)
  dist = torch.hypot(cp_xy[:, :, 0] - state.x, cp_xy[:, :, 1] - state.y)
  return state.pods[:, :, 5] - (dist / 6000.0)

def extract_features(state: State) -> torch.Tensor:
  px, py = state.x, state.y
  pvx, pvy = state.vx, state.vy
  pang = state.angle

  fx, fy = torch.cos(pang), torch.sin(pang)

  slide_x = px + 5.6667 * pvx
  slide_y = py + 5.6667 * pvy

  progress = compute_progress(state) # [B, 4]
  timeout_0 = state.timeouts[:, 0] / 100.0

  pod_feature_list = []
  for p in (0, 1):
    f_x, f_y = fx[:, p], fy[:, p]
    vx_c = (f_x * pvx[:, p] + f_y * pvy[:, p]) / 600.0
    vy_c = (f_x * pvy[:, p] - f_y * pvx[:, p]) / 600.0
    spd = torch.hypot(pvx[:, p], pvy[:, p]) / 600.0
    shld = state.shield[:, p] / 3.0
    bst = 1.0 - state.boosted[:, p]
    self_feats = torch.stack([vx_c, vy_c, spd, shld, bst, timeout_0], dim=-1)
    
    cp_feats = []
    for k in (0, 1, 2):
      cp_xy = get_checkpoint_xy(state, state.next_cp[:, p] + k)
      dx, dy = cp_xy[:, 0] - px[:, p], cp_xy[:, 1] - py[:, p]
      dist = torch.hypot(dx, dy) + 1e-5
      cos_cp = (f_x * dx + f_y * dy) / dist
      sin_cp = (f_x * dy - f_y * dx) / dist
      dist_norm = torch.tanh(dist / 5000.0)

      sdx, sdy = cp_xy[:, 0] - slide_x[:, p], cp_xy[:, 1] - slide_y[:, p]
      sdist = torch.hypot(sdx, sdy) + 1e-5
      cos_slide = (f_x * sdx + f_y * sdy) / sdist
      sin_slide = (f_x * sdy - f_y * sdx) / sdist
      cp_feats.append(
        torch.stack([cos_cp, sin_cp, dist_norm, cos_slide, sin_slide], dim=-1)
      )

    rel_feats = []
    for xor_mask in (1, 2, 3):
      o = p ^ xor_mask
      dx, dy = px[:, o] - px[:, p], py[:, o] - py[:, p]
      d = torch.hypot(dx, dy) + 1e-5
      cos_o = (f_x * dx + f_y * dy) / d
      sin_o = (f_x * dy - f_y * dx) / d
      prox = torch.exp(-d / 1500.0)
      dvx, dvy = pvx[:, o] - pvx[:, p], pvy[:, o] - pvy[:, p]
      rel_vx = torch.tanh((f_x * dvx + f_y * dvy) / 600.0)
      rel_feats.append(torch.stack([cos_o, sin_o, prox, rel_vx], dim=-1))

    is_leader = torch.where(
      progress[:, p] >= progress[:, p ^ 1], 1.0, -1.0
    ).unsqueeze(-1)

    pod_feature_list.append(
      torch.cat([self_feats, *cp_feats, *rel_feats, is_leader], dim=-1)
    )

  return torch.cat(pod_feature_list, dim=-1)

def layer_init(layer, std=np.sqrt(2), bias_const=0.0):
  torch.nn.init.orthogonal_(layer.weight, std)
  torch.nn.init.constant_(layer.bias, bias_const)
  return layer

class Agent(nn.Module):
  def __init__(
    self, 
    state_dim: int, 
    device: torch.Device):
    super().__init__()
    self.device = device
    self.register_buffer(
      "angle_table",
      torch.linspace(-MAX_ROTATION, +MAX_ROTATION, 3, device=device),
    )
    self.register_buffer(
      "thrust_table",
      torch.tensor([0.0, 200.0, 0.0, 200.0], device=device),
    )

    self.critic = nn.Sequential(
      layer_init(nn.Linear(state_dim, 256)),
      nn.LayerNorm(256),
      nn.SiLU(),
      layer_init(nn.Linear(256, 256)),
      nn.LayerNorm(256),
      nn.SiLU(),
      layer_init(nn.Linear(256, 1), std=1.0)
    )

    self.actor_trunk = nn.Sequential(
      layer_init(nn.Linear(state_dim, 128)),
      nn.LayerNorm(128),
      nn.SiLU(),
      layer_init(nn.Linear(128, 128)),
      nn.LayerNorm(128),
      nn.SiLU()
    )

    self.head_ang0 = layer_init(nn.Linear(128, 3), std=0.01)
    self.head_thr0 = layer_init(nn.Linear(128, 4), std=0.01)
    self.head_ang1 = layer_init(nn.Linear(128, 3), std=0.01)
    self.head_thr1 = layer_init(nn.Linear(128, 4), std=0.01)

  def get_value(self, obs: torch.Tensor) -> torch.Tensor
    return self.critic(obs).squeeze(-1)

  def get_action_and_value(
    self,
    obs: torch.Tensor,
    actions: torch.Tensor | None = None
  ):
    h = self.actor_trunk(obs)
    dists = [
      Categorical(logits=self.head_ang0(h)),
      Categorical(logits=self.head_thr0(h)),
      Categorical(logits=self.head_ang1(h)),
      Categorical(logits=self.head_thr1(h)),
    ]
    if actions is None:
      actions = torch.stack([d.sample() for d in dists], dim=-1)

    logprob = sum(dists[i].log_prob(actions[:, i]) for i in range(4))
    entropy = sum(d.entropy() for d in dists)
    value = self.critic(obs).squeeze(-1)
    return actions, logprob, entropy, value

  def decode_actions(
