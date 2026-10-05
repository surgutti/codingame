import math
import torch
import torch.nn as nn
from state import State, MAX_ROT

PI, FRICTION, MAX_THRUST, CP_R = math.pi, 0.85, 200.0, 600.0
K_DRIFT, A_CUT, EARLY_N, EARLY_T = 3.4, 1.28, 8, 0.98


def _cp(state: State, idx: torch.Tensor) -> torch.Tensor:
    B, E, P = idx.shape
    mod = (idx % state.num_cps.unsqueeze(-1)).unsqueeze(-1).expand(B, E, P, 2)
    return torch.gather(state.checkpoints, dim=2, index=mod)


def _steer(x, y, vx, vy, ang, tx, ty, k=K_DRIFT):
    diff = (torch.atan2(ty - k * vy - y, tx - k * vx - x) - ang + PI) % (2.0 * PI) - PI
    return diff.clamp(-MAX_ROT, MAX_ROT), diff, (diff.abs() - MAX_ROT).clamp_min(0.0)


def _thrust(res):
    return MAX_THRUST * torch.cos(res.clamp_max(PI / 2.0)) * (res < A_CUT).float()


def _passes(x, y, nx, ny, cx, cy):
    dx, dy, wx, wy = nx - x, ny - y, cx - x, cy - y
    t = ((wx * dx + wy * dy) / (dx * dx + dy * dy + 1e-6)).clamp(0.0, 1.0)
    return (wx - t * dx) ** 2 + (wy - t * dy) ** 2 < CP_R * CP_R


@torch.no_grad()
@torch.compile(fullgraph=True)
def apex_formula_act(state: State) -> torch.Tensor:
    B, E, _ = state.raw.shape
    x, y, vx, vy, ang, ncp = state.x, state.y, state.vx, state.vy, state.angle, state.next_cp
    rx, ry, rvx, rvy, rang, rncp = x[..., :2], y[..., :2], vx[..., :2], vy[..., :2], ang[..., :2], ncp[..., :2]
    sh_cd = state.shield[..., :2]

    # 1. Runner: Out-in-Out Entry Swing (160u) + 8-Turn Multi-CP Coasting Lookahead + Shield Pre-Rotate
    c0, c1, c2 = _cp(state, rncp), _cp(state, rncp + 1), _cp(state, rncp + 2)
    is_final_cp = rncp >= (state.laps * state.num_cps).unsqueeze(-1)
    dist = torch.sqrt((c0[..., 0] - rx) ** 2 + (c0[..., 1] - ry) ** 2 + 1e-6)
    u01_x, u01_y = c1[..., 0] - c0[..., 0], c1[..., 1] - c0[..., 1]
    u01_d = torch.sqrt(u01_x * u01_x + u01_y * u01_y + 1e-6)
    swing = ((dist - 1800.0) / 3000.0).clamp(0.0, 1.0) * 160.0
    c0_aim_x, c0_aim_y = c0[..., 0] - swing * (u01_x / u01_d), c0[..., 1] - swing * (u01_y / u01_d)

    sx, sy, svx, svy, sang, scd = rx, ry, rvx, rvy, rang, sh_cd
    early = torch.zeros_like(rx, dtype=torch.bool)
    for _ in range(EARLY_N):
        tx_h, ty_h = torch.where(early, c2[..., 0], c1[..., 0]), torch.where(early, c2[..., 1], c1[..., 1])
        rot_h, _, res_h = _steer(sx, sy, svx, svy, sang, tx_h, ty_h)
        sang = sang + rot_h
        thr_h = torch.where(scd > 0.5, torch.zeros_like(res_h), EARLY_T * _thrust(res_h))
        scd = (scd - 1.0).clamp_min(0.0)
        svx, svy = svx + thr_h * torch.cos(sang), svy + thr_h * torch.sin(sang)
        nx, ny = sx + svx, sy + svy
        early |= _passes(sx, sy, nx, ny, c0[..., 0], c0[..., 1])
        sx, sy, svx, svy = nx, ny, svx * FRICTION, svy * FRICTION
    early = early & (~is_final_cp)

    tx, ty = torch.where(early, c1[..., 0], c0_aim_x), torch.where(early, c1[..., 1], c0_aim_y)
    rot, diff, res = _steer(rx, ry, rvx, rvy, rang, tx, ty)
    thrust = torch.where(early, EARLY_T * _thrust(res), _thrust(res))
    rot_c1, _, _ = _steer(rx, ry, rvx, rvy, rang, c1[..., 0], c1[..., 1])
    rot = torch.where((sh_cd > 0.5) & (dist < 1800.0), rot_c1, rot)
    boost = (state.boosted[..., :2] < 0.5) & (dist > 3200.0) & (diff.abs() < 0.20) & (~early)

    px, py = (rx + rvx).unsqueeze(-1), (ry + rvy).unsqueeze(-1)
    ox, oy = (x[..., 2:] + vx[..., 2:]).unsqueeze(-2), (y[..., 2:] + vy[..., 2:]).unsqueeze(-2)
    dvx, dvy = rvx.unsqueeze(-1) - vx[..., 2:].unsqueeze(-2), rvy.unsqueeze(-1) - vy[..., 2:].unsqueeze(-2)
    shield = (((px - ox) ** 2 + (py - oy) ** 2 < 820.0 ** 2) & (dvx * dvx + dvy * dvy > 500.0 ** 2)).any(-1)
    runner_acts = torch.stack([rot, thrust, shield.float(), boost.float()], dim=-1)

    # 2. Predator Blocker: Velocity-Projected Role + 2-CP Ambush + Gatekeeper Guard + Lead-Pursuit Strike
    cps_all = _cp(state, ncp)
    dist_all = torch.sqrt((cps_all[..., 0] - (x + 2.0 * vx)) ** 2 + (cps_all[..., 1] - (y + 2.0 * vy)) ** 2 + 1e-6)
    prog = ncp.float() * 36000.0 - dist_all
    p0_is_runner, e2_is_leader = prog[..., 0] >= prog[..., 1], prog[..., 2] >= prog[..., 3]

    ex, ey = torch.where(e2_is_leader, x[..., 2], x[..., 3]), torch.where(e2_is_leader, y[..., 2], y[..., 3])
    evx, evy = torch.where(e2_is_leader, vx[..., 2], vx[..., 3]), torch.where(e2_is_leader, vy[..., 2], vy[..., 3])
    eang, encp = torch.where(e2_is_leader, ang[..., 2], ang[..., 3]), torch.where(e2_is_leader, ncp[..., 2], ncp[..., 3])
    edist = torch.where(e2_is_leader, dist_all[..., 2], dist_all[..., 3])

    ecps = _cp(state, torch.stack([encp, encp + 1], dim=-1))
    ecp0_x, ecp0_y, ecp1_x, ecp1_y = ecps[..., 0, 0], ecps[..., 0, 1], ecps[..., 1, 0], ecps[..., 1, 1]
    d_b_cp0 = torch.sqrt((ecp0_x.unsqueeze(-1) - (rx + 2.0 * rvx)) ** 2 + (ecp0_y.unsqueeze(-1) - (ry + 2.0 * rvy)) ** 2 + 1e-6)
    use_cp1 = d_b_cp0 > (edist.unsqueeze(-1) + 320.0)
    t_cpx, t_cpy = torch.where(use_cp1, ecp1_x.unsqueeze(-1), ecp0_x.unsqueeze(-1)), torch.where(use_cp1, ecp1_y.unsqueeze(-1), ecp0_y.unsqueeze(-1))

    u_ex, u_ey = (ex.unsqueeze(-1) + 2.0 * evx.unsqueeze(-1)) - t_cpx, (ey.unsqueeze(-1) + 2.0 * evy.unsqueeze(-1)) - t_cpy
    u_enorm = torch.sqrt(u_ex * u_ex + u_ey * u_ey + 1e-6)
    guard_x, guard_y = t_cpx + 520.0 * (u_ex / u_enorm), t_cpy + 520.0 * (u_ey / u_enorm)

    d_b_enemy = torch.sqrt((ex.unsqueeze(-1) - rx) ** 2 + (ey.unsqueeze(-1) - ry) ** 2 + 1e-6)
    d_b_guard = torch.sqrt((guard_x - rx) ** 2 + (guard_y - ry) ** 2 + 1e-6)
    rel_speed = torch.sqrt((evx.unsqueeze(-1) - rvx) ** 2 + (evy.unsqueeze(-1) - rvy) ** 2 + 1e-6)
    t_lead = ((d_b_enemy - 450.0).clamp_min(0.0) / (rel_speed + 320.0)).clamp(0.4, 4.5)
    e_to_cp_x, e_to_cp_y = ecp0_x - ex, ecp0_y - ey
    e_to_cp_d = torch.sqrt(e_to_cp_x * e_to_cp_x + e_to_cp_y * e_to_cp_y + 1e-6)
    ram_x = ex.unsqueeze(-1) + t_lead * (evx.unsqueeze(-1) + 80.0 * t_lead * (e_to_cp_x / e_to_cp_d).unsqueeze(-1))
    ram_y = ey.unsqueeze(-1) + t_lead * (evy.unsqueeze(-1) + 80.0 * t_lead * (e_to_cp_y / e_to_cp_d).unsqueeze(-1))

    strike = ((d_b_enemy < 3400.0) & ((~use_cp1) | (d_b_guard < 1400.0))) | (d_b_guard < 850.0)
    aim_x, aim_y, k_use = torch.where(strike, ram_x, guard_x), torch.where(strike, ram_y, guard_y), torch.where(strike, 2.6, 3.3)
    diff_b = (torch.atan2(aim_y - k_use * rvy - ry, aim_x - k_use * rvx - rx) - rang + PI) % (2.0 * PI) - PI
    brot, bres = diff_b.clamp(-MAX_ROT, MAX_ROT), (diff_b.abs() - MAX_ROT).clamp_min(0.0)
    bthr = torch.where(strike & (bres < 0.55), torch.full_like(bres, MAX_THRUST), _thrust(bres))

    waiting = (~strike) & (d_b_guard < 1050.0)
    w_rot, _, _ = _steer(rx, ry, rvx, rvy, rang, ram_x, ram_y, k=0.4)
    brot, bthr = torch.where(waiting, w_rot, brot), torch.where(waiting, torch.zeros_like(bthr), bthr)

    b_nx, b_ny = rx + rvx + bthr * torch.cos(rang + brot), ry + rvy + bthr * torch.sin(rang + brot)
    e_nx, e_ny = ex.unsqueeze(-1) + evx.unsqueeze(-1) + 150.0 * torch.cos(eang).unsqueeze(-1), ey.unsqueeze(-1) + evy.unsqueeze(-1) + 150.0 * torch.sin(eang).unsqueeze(-1)
    bsh = (((b_nx - e_nx) ** 2 + (b_ny - e_ny) ** 2 < 835.0 ** 2) & (rel_speed > 200.0)).float()

    mate_nx, mate_ny = b_nx.flip(-1), b_ny.flip(-1)
    near_mate = ((b_nx - mate_nx) ** 2 + (b_ny - mate_ny) ** 2 < 920.0 ** 2)
    bsh, bthr = torch.where(near_mate, torch.zeros_like(bsh), bsh), torch.where(near_mate, torch.zeros_like(bthr), bthr)
    away_rot, _, _ = _steer(rx, ry, rvx, rvy, rang, 2.0 * rx - mate_nx, 2.0 * ry - mate_ny, k=1.0)
    brot = torch.where(near_mate, away_rot, brot)

    blocker_acts = torch.stack([brot, bthr, bsh, torch.zeros_like(bsh)], dim=-1)
    is_runner_mask = (torch.stack([p0_is_runner, ~p0_is_runner], dim=-1) | is_final_cp | (state.timeouts[..., 0:1] < 65.0)).unsqueeze(-1)
    return torch.where(is_runner_mask, runner_acts, blocker_acts).view(B, E, 8)

class Handmade2Agent(nn.Module):
  def __init__(self):
    super().__init__()
  def act(self, state, training=False):
    return apex_formula_act(state)
  def observe(self, reward, next_state, done):
    return
  def update(self):
    return {}
