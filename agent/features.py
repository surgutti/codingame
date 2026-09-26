import torch
from state import State, get_checkpoint_xy

def compute_progress(state: State) -> torch.Tensor:
  cp_xy = get_checkpoint_xy(state, state.next_cp)
  dist = torch.hypot(cp_xy[..., 0] - state.x, cp_xy[..., 1] - state.y)
  return state.next_cp.float() - (dist / 6000.0)

def extract_features(state: State) -> torch.Tensor:
  px, py = state.x, state.y
  pvx, pvy = state.vx, state.vy
  pang = state.angle

  fx, fy = torch.cos(pang), torch.sin(pang)

  slide_x = px + 5.6667 * pvx
  slide_y = py + 5.6667 * pvy

  progress = compute_progress(state) # [B, 4]
  timeout_0 = state.timeouts[..., 0] / 100.0
  timeout_1 = state.timeouts[..., 0] / 100.0

  pod_feature_list = []
  for p in (0, 1):
    f_x, f_y = fx[..., p], fy[..., p]
    vx_c = (f_x * pvx[..., p] + f_y * pvy[..., p]) / 600.0
    vy_c = (f_x * pvy[..., p] - f_y * pvx[..., p]) / 600.0
    spd = torch.hypot(pvx[..., p], pvy[..., p]) / 600.0
    shld = state.shield[..., p] / 3.0
    bst = 1.0 - state.boosted[..., p]
    self_feats = torch.stack([vx_c, vy_c, spd, shld, bst, timeout_0, timeout_1], dim=-1)
    
    cp_feats = []
    for k in (0, 1, 2):
      cp_xy = get_checkpoint_xy(state, state.next_cp[..., p] + k)
      dx, dy = cp_xy[..., 0] - px[..., p], cp_xy[..., 1] - py[..., p]
      dist = torch.hypot(dx, dy) + 1e-5
      cos_cp = (f_x * dx + f_y * dy) / dist
      sin_cp = (f_x * dy - f_y * dx) / dist
      dist_norm = torch.tanh(dist / 5000.0)

      sdx, sdy = cp_xy[..., 0] - slide_x[..., p], cp_xy[..., 1] - slide_y[..., p]
      sdist = torch.hypot(sdx, sdy) + 1e-5
      cos_slide = (f_x * sdx + f_y * sdy) / sdist
      sin_slide = (f_x * sdy - f_y * sdx) / sdist
      cp_feats.append(
        torch.stack([cos_cp, sin_cp, dist_norm, cos_slide, sin_slide], dim=-1)
      )

    rel_feats = []
    for xor_mask in (1, 2, 3):
      o = p ^ xor_mask
      dx, dy = px[..., o] - px[..., p], py[..., o] - py[..., p]
      d = torch.hypot(dx, dy) + 1e-5
      cos_o = (f_x * dx + f_y * dy) / d
      sin_o = (f_x * dy - f_y * dx) / d
      prox = torch.exp(-d / 1500.0)
      dvx, dvy = pvx[..., o] - pvx[..., p], pvy[..., o] - pvy[..., p]
      rel_vx = torch.tanh((f_x * dvx + f_y * dvy) / 600.0)
      rel_feats.append(torch.stack([cos_o, sin_o, prox, rel_vx], dim=-1))

    is_leader = torch.where(
      progress[..., p] >= progress[..., p ^ 1], 1.0, -1.0
    ).unsqueeze(-1)

    pod_feature_list.append(
      torch.cat([self_feats, *cp_feats, *rel_feats, is_leader], dim=-1)
    )

  return torch.cat(pod_feature_list, dim=-1)
