#!/usr/bin/env python3

import math
import torch
import torch.nn as nn
from state import State, MAX_ROT

PI = math.pi
FRICTION = 0.85; MAX_THRUST = 200.0; CP_R = 600.0
K_DRIFT = 3.4            # aim at target - K * velocity (momentum compensation)
A_CUT = 1.28             # residual heading error (rad) at which thrust hits 0
EARLY_N = 7              # rollout horizon (turns) for the "turn early" test
EARLY_T = 0.95           # thrust fraction while already turning toward the next checkpoint
BOOST_D = 3200.0; BOOST_ANG = 0.20
SHIELD_V = 500.0

def _cp(state, idx):                       # idx [B,E,P] -> xy [B,E,P,2], wraps the lap
  B, E, P = idx.shape
  mod = (idx % state.num_cps.unsqueeze(-1)).unsqueeze(-1).expand(B, E, P, 2)
  return torch.gather(state.checkpoints, dim=2, index=mod)

def _steer(x, y, vx, vy, ang, tx, ty, k=K_DRIFT):
  diff = (torch.atan2(ty - k * vy - y, tx - k * vx - x) - ang + PI) % (2.0 * PI) - PI
  return diff.clamp(-MAX_ROT, MAX_ROT), diff, (diff.abs() - MAX_ROT).clamp_min(0.0)

def _thrust(res):
  return MAX_THRUST * torch.cos(res.clamp_max(PI / 2)) * (res < A_CUT).float()

def _passes(x, y, nx, ny, cx, cy):         # segment-circle test, same as the engine
  dx, dy, wx, wy = nx - x, ny - y, cx - x, cy - y
  t = ((wx * dx + wy * dy) / (dx * dx + dy * dy + 1e-6)).clamp(0.0, 1.0)
  return (wx - t * dx) ** 2 + (wy - t * dy) ** 2 < CP_R * CP_R

@torch.no_grad()
@torch.compile(fullgraph=True)
def racer_act(state: State) -> torch.Tensor:
  B, E, _ = state.raw.shape
  x, y, vx, vy, ang = state.x[..., :2], state.y[..., :2], state.vx[..., :2], state.vy[..., :2], state.angle[..., :2]
  c0, c1 = _cp(state, state.next_cp[..., :2]), _cp(state, state.next_cp[..., :2] + 1)
  rx, ry = c0[..., 0] - x, c0[..., 1] - y
  dist = torch.sqrt(rx * rx + ry * ry)
  # 1. Early-turn test: simulate "already steering to the next cp" for EARLY_N turns;
  #    if that trajectory still crosses the current cp, start turning now.
  sx, sy, svx, svy, sang = x, y, vx, vy, ang
  early = torch.zeros_like(x, dtype=torch.bool)
  for _ in range(EARLY_N):
    rot, _, res = _steer(sx, sy, svx, svy, sang, c1[..., 0], c1[..., 1])
    sang = sang + rot
    thr = EARLY_T * _thrust(res)
    svx, svy = svx + thr * torch.cos(sang), svy + thr * torch.sin(sang)
    nx, ny = sx + svx, sy + svy
    early |= _passes(sx, sy, nx, ny, c0[..., 0], c0[..., 1])
    sx, sy, svx, svy = nx, ny, svx * FRICTION, svy * FRICTION
  # 2. Steer + thrust toward the chosen checkpoint.
  tx, ty = torch.where(early, c1[..., 0], c0[..., 0]), torch.where(early, c1[..., 1], c0[..., 1])
  rot, diff, res = _steer(x, y, vx, vy, ang, tx, ty)
  thrust = torch.where(early, EARLY_T * _thrust(res), _thrust(res))
  # 3. Boost once on a long, aligned straight.
  boost = (state.boosted[..., :2] < 0.5) & (dist > BOOST_D) & (diff.abs() < BOOST_ANG) & ~early
  # 4. Shield on predicted hard enemy overlap next turn.
  px, py = (x + vx).unsqueeze(-1), (y + vy).unsqueeze(-1)
  ox, oy = (state.x[..., 2:] + state.vx[..., 2:]).unsqueeze(-2), (state.y[..., 2:] + state.vy[..., 2:]).unsqueeze(-2)
  dvx, dvy = vx.unsqueeze(-1) - state.vx[..., 2:].unsqueeze(-2), vy.unsqueeze(-1) - state.vy[..., 2:].unsqueeze(-2)
  shield = (((px - ox) ** 2 + (py - oy) ** 2 < 820.0 ** 2) & (dvx * dvx + dvy * dvy > SHIELD_V ** 2)).any(-1)
  # 5. Teammate avoidance: the pod further from its checkpoint yields.
  mate = ((px[..., 0, 0] - px[..., 1, 0]) ** 2 + (py[..., 0, 0] - py[..., 1, 0]) ** 2 < 820.0 ** 2).unsqueeze(-1)
  thrust = torch.where(mate & (dist > dist.flip(-1)), torch.zeros_like(thrust), thrust)
  return torch.stack([rot, thrust, shield.float(), boost.float()], dim=-1).view(B, E, 8)

class HandmadeAgent(nn.Module):
  def __init__(self):
    super().__init__()
  def act(self, state, training=False):
    return racer_act(state)
  def observe(self, reward, next_state, done):
    return
  def update(self):
    return {}
