#!/usr/bin/env python3
import torch
import torch.nn as nn
import torch.nn.functional as F

@torch.compile(fullgraph=True)
def _fast_sample_and_logprob(
  logits: torch.Tensor,
  action_list: torch.Tensor,
) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor]:
  log_probs = F.log_softmax(logits, dim=-1)
  u = torch.empty_like(logits).exponential_()
  action_idx = (logits - u.log()).argmax(dim=-1)
  chosen_logprob = log_probs.gather(-1, action_idx.unsqueeze(-1))
  action = action_list[action_idx]
  return action, action_idx.unsqueeze(-1), chosen_logprob

@torch.compile(fullgraph=True)
def _compute_gae_fused(
  rewards: torch.Tensor,
  values: torch.Tensor,
  next_values: torch.Tensor,
  is_done: torch.Tensor,
  gamma: float,
  gae_lambda: float,
):
  T = rewards.shape[0]
  not_done = 1.0 - is_done
  delta = rewards + gamma * next_values * not_done - values
  advantages = torch.zeros_like(delta)
  A = torch.zeros_like(values[0])
  gl = gamma * gae_lambda
  for i in range(T - 1, -1, -1):
    A = delta[i] + gl * not_done[i] * A
    advantages[i] = A
  
  target_values = values + advantages
  return advantages, target_values, delta

# C^1-Smooth Hybrid SPO
# Dual-Clip
# Huberized-Gain Surrogate Objective
@torch.compile(fullgraph=True)
def compute_spo_dual_clip_loss(
    logits: torch.Tensor,
    mb_actions: torch.Tensor,
    mb_old_logprobs: torch.Tensor,
    mb_advantages: torch.Tensor,
    clip_eps_low: float = 0.20,
    clip_eps_high: float = 0.28,
    dual_clip_c: float = 3.0,
    ratio_cap: float = 3.0,
) -> tuple[
    torch.Tensor,
    torch.Tensor,
    torch.Tensor,
    torch.Tensor,
    torch.Tensor,
    torch.Tensor,
    torch.Tensor,
    torch.Tensor,
]:
  log_probs = F.log_softmax(logits, dim=-1)
  probs = torch.exp(log_probs)
  mb_logprobs = log_probs.gather(-1, mb_actions)

  log_ratio = (mb_logprobs - mb_old_logprobs).clamp(-10.0, 10.0)
  ratio = torch.exp(log_ratio)

  r_pos_det = ratio.detach().clamp(min=1.0 - ratio_cap, max=1.0 + ratio_cap)
  pos_slope = mb_advantages * (1.0 - (r_pos_det - 1.0) / clip_eps_high)
  pos_val = (
      ratio * mb_advantages
      - (mb_advantages.abs() / (2.0 * clip_eps_high))
      * ((r_pos_det - 1.0).pow(2))
  ).detach()
  pos_obj = ratio * pos_slope + (pos_val - (ratio * pos_slope).detach())

  r_neg_det = ratio.detach().clamp(min=1.0 - ratio_cap, max=1.0)
  neg_spo_slope = mb_advantages.abs() * (
      -1.0 - (r_neg_det - 1.0) / clip_eps_low
  )
  neg_spo_val = (
      ratio * mb_advantages
      - (mb_advantages.abs() / (2.0 * clip_eps_low))
      * ((r_neg_det - 1.0).pow(2))
  ).detach()
  neg_spo_obj = ratio * neg_spo_slope + (
      neg_spo_val - (ratio * neg_spo_slope).detach()
  )
  neg_dc_obj = torch.max(ratio * mb_advantages, dual_clip_c * mb_advantages)
  neg_obj = torch.where(ratio <= 1.0, neg_spo_obj, neg_dc_obj)

  pos_mask = mb_advantages >= 0.0
  obj = torch.where(pos_mask, pos_obj, neg_obj)
  L_spo = -obj.mean()

  entropy = -(probs * log_probs).sum(dim=-1).mean()
  max_prob = probs.max(dim=-1).values.mean()
  approx_kl = ((ratio - 1.0) - log_ratio).mean()

  spo_restoring_mask = (pos_mask & (ratio > 1.0 + clip_eps_high)) | (
      (~pos_mask) & (ratio < 1.0 - clip_eps_low)
  )
  dual_clip_mask = (~pos_mask) & (ratio > dual_clip_c)
  clip_frac = (spo_restoring_mask | dual_clip_mask).float().mean()
  spo_restoring_frac = spo_restoring_mask.float().mean()
  dual_clip_frac = dual_clip_mask.float().mean()

  # Kong's effective sample size ess
  mean_r = ratio.mean()
  ess = (mean_r * mean_r) / (ratio * ratio).mean().clamp_min(1e-12)

  return (
      L_spo,
      entropy,
      max_prob,
      approx_kl,
      clip_frac,
      spo_restoring_frac,
      dual_clip_frac,
      ess,
  )

class RunningReturnScaler(nn.Module):
  def __init__(self, beta: float = 0.01, eps: float = 1e-4):
    super().__init__()
    self.beta = beta
    self.eps = eps
    self.register_buffer("mean", torch.zeros(1))
    self.register_buffer("var", torch.ones(1))
    self.register_buffer("initialized", torch.tensor(False))

  @torch.no_grad()
  def update(self, returns: torch.Tensor) -> float:
    b_mean = returns.mean()
    b_var = returns.var(unbiased=False).clamp_min(self.eps)
    if not bool(self.initialized.item()):
      self.mean.copy_(b_mean.view(1))
      self.var.copy_(b_var.view(1))
      self.initialized.fill_(True)
    else:
      self.mean.lerp_(b_mean.view(1), self.beta)
      self.var.lerp_(b_var.view(1), self.beta)
    return float(torch.sqrt(self.var + self.eps).item())
