#!/usr/bin/env python3
import math
from pathlib import Path

import torch
import torch.nn as nn
import torch.nn.functional as F
from torch.distributions.categorical import Categorical

from state import State
from spo.model import ActorNetwork, CriticNetwork
from spo.config import SPOConfig
from spo.spo_utils import _fast_sample_and_logprob, compute_spo_dual_clip_loss, _compute_gae_fused, RunningReturnScaler

class SPOAgent(nn.Module):
  def __init__(self, config: SPOConfig):
    super().__init__()
    self.config = config
    self.batch_dim = (config.episode_steps, config.num_envs)
    self.device = config.device
    
    MAX_ROT = 0.3141592653589793
    angles = torch.tensor([[-MAX_ROT], [0.0], [+MAX_ROT]])
    thrusts = torch.tensor([
        [0.0, 0.0, 0.0],    # 0: thrust 0
        [200.0, 0.0, 0.0],  # 1: thrust 200
        [0.0, 0.0, 1.0],    # 2: BOOST
        [0.0, 1.0, 0.0],    # 3: SHIELD
    ])

    pod_action = torch.cat(
        [angles.repeat(4, 1), thrusts.repeat_interleave(3, dim=0)], dim=1
    )  # (12, 4)

    action_list = torch.cat(
        [pod_action.repeat_interleave(12, dim=0), pod_action.repeat(12, 1)],
        dim=1,
    ).to(self.device)  # (144, 8)
    # print(f"{action_list=}")

    self.register_buffer("action_list", action_list)

    state_dim = config.state_dim
    action_dim = len(self.action_list)

    self.state_dim = state_dim
    self.action_dim = action_dim
    self.entropy_norm_const = math.log(action_dim) # ln(144) = 4.9698133

    self.actor = torch.compile(ActorNetwork(action_dim).to(self.device))
    self.critic = torch.compile(CriticNetwork().to(self.device))

    self.optimizer = torch.optim.AdamW(
      [ 
        {'params': self.critic.parameters()},
        {'params': self.actor.parameters()}
      ],
      lr=config.learning_rate,
      eps=1e-5,
      fused=(self.device == "cuda")
    )

    self.return_scaler = RunningReturnScaler(
      beta=0.01
    ).to(self.device)

    self.batch_idx = 0
    self.states = torch.zeros((*self.batch_dim, state_dim), dtype=torch.float32, device=self.device)
    self.actions = torch.zeros((*self.batch_dim, 1), dtype=torch.long, device=self.device)
    self.rewards = torch.zeros((*self.batch_dim, 1), dtype=torch.float32, device=self.device)
    self.dones = torch.zeros((*self.batch_dim, 1), dtype=torch.long, device=self.device)
    self.logprobs = torch.zeros((*self.batch_dim, 1), dtype=torch.float32, device=self.device)
    self.values = torch.zeros((*self.batch_dim, 1), dtype=torch.float32, device=self.device)
    self.next_values = torch.zeros((*self.batch_dim, 1), dtype=torch.float32, device=self.device)
    self.last_next_state = torch.zeros((config.num_envs, state_dim), dtype=torch.float32, device=self.device)


  def get_value(self, state: torch.Tensor) -> torch.Tensor:
    return self.critic(state)

  def act_dist(self, state: torch.Tensor) -> Categorical:
    return Categorical(
      logits=self.actor(self.state),
      validate_args=False
    )

  @torch.no_grad()
  def act(
    self,
    state,
    training: bool = False
  ):
    s = state.raw.squeeze(0)
    logits = self.actor(s)
    action, action_idx, chosen_logprob = _fast_sample_and_logprob(
      logits, self.action_list
    )

    if training:
      idx = self.batch_idx
      self.states[idx].copy_(s)
      self.actions[idx].copy_(action_idx)
      self.logprobs[idx].copy_(chosen_logprob)

    return action.unsqueeze(0)
  
  @torch.no_grad()
  def observe(
    self,
    reward: torch.Tensor,
    next_state: State,
    done: torch.Tensor
  ):
    idx = self.batch_idx
    self.rewards[idx].copy_(reward.view(-1, 1))
    self.dones[idx].copy_(done.view(-1, 1).long())

    if idx == self.batch_dim[0] - 1:
      self.last_next_state.copy_(next_state.raw.squeeze(0))

    self.batch_idx += 1
  
  def parameter_count(self) -> int:
    return sum(p.numel() for p in self.parameters())

  def save(self, path: str | Path) -> None:
    torch.save(self.state_dict(), path)

  def load(self, path: str | Path) -> None:
    ckpt = torch.load(path, map_location=self.config.device, weights_only=False)
    self.load_state_dict(ckpt)

  @torch.no_grad()
  def _evaluate_rollout_values(self) -> None:
    T, E = self.batch_dim
    flat_states = self.states.flatten(0, 1)
    all_states = torch.cat([flat_states, self.last_next_state], dim=0)
    all_values = self.critic(all_states)

    vals_te = all_values[: T * E].view(T, E, 1)
    last_val = all_values[T * E :].view(E, 1)

    self.values.copy_(vals_te)
    if T > 1:
      self.next_values[:-1].copy_(vals_te[1:])
    self.next_values[-1].copy_(last_val)

  def update(self) -> dict[str, float]:
    T, E = self.batch_dim
    B = T * E

    assert (
      self.batch_idx == T
    ), f"Expected batch_idx == {T}, got {self.batch_idx}"
    self.batch_idx = 0

    self._evaluate_rollout_values()

    is_done = (self.dones > 0).float()
    advantages, target_values, delta = _compute_gae_fused(
      self.rewards,
      self.values,
      self.next_values,
      is_done,
      gamma=self.config.gamma,
      gae_lambda=self.config.gae_lambda
    )

    raw_adv_std = advantages.std(unbiased=False)

    y_pred = self.values.flatten()
    y_true = target_values.flatten()
    var_y = torch.var(y_true, unbiased=False)
    explained_var = 1.0 - torch.var(y_true - y_pred, unbiased=False) / (var_y + 1e-8)

    return_scale_std = self.return_scaler.update(target_values)

    act_flat = self.actions.flatten()
    pod0_act = torch.div(act_flat, 12, rounding_mode="floor")
    pod1_act = act_flat % 12
    all_pods_acts = torch.cat([pod0_act, pod1_act], dim=0)
    steer_idx = all_pods_acts % 3
    thrust_idx = torch.div(all_pods_acts, 3, rounding_mode="floor")

    if self.config.norm_adv:
      advantages = (advantages - advantages.mean()) / (
        advantages.std(unbiased=False) + 1e-8
      )
      advantages = self.config.adv_tail_c * torch.asinh(advantages / self.config.adv_tail_c)
      advantages = (advantages - advantages.mean()) / (
        advantages.std(unbiased=False) + 1e-8
      )

    states = self.states.flatten(0, 1)
    actions = self.actions.flatten(0, 1)
    dones = self.dones.flatten(0, 1)
    logprobs = self.logprobs.flatten(0, 1)
    values = self.values.flatten(0, 1)
    target_values_flat = target_values.flatten(0, 1)
    advantages_flat = advantages.flatten(0, 1)

    L_spo_acc = torch.zeros((), device=self.device)
    L_vf_acc = torch.zeros((), device=self.device)
    S_pi_acc = torch.zeros((), device=self.device)
    approx_kl_acc = torch.zeros((), device=self.device)
    clip_frac_acc = torch.zeros((), device=self.device)
    spo_restoring_acc = torch.zeros((), device=self.device)
    dual_clip_acc = torch.zeros((), device=self.device)
    ess_acc = torch.zeros((), device=self.device)
    max_prob_acc = torch.zeros((), device=self.device)
    actor_grad_acc = torch.zeros((), device=self.device)
    critic_grad_acc = torch.zeros((), device=self.device)
    total_batches = 0
    actor_batches = 0

    for epoch in range(self.config.update_epochs): 
      inds = torch.randperm(B, device=self.device)
      do_actor_step = True

      for start in range(0, B, self.config.minibatch_size):
        end = min(start + self.config.minibatch_size, B)
        mb_idx = inds[start:end]

        mb_states = states[mb_idx]
        mb_actions = actions[mb_idx]
        mb_old_logprobs = logprobs[mb_idx]
        mb_old_values = values[mb_idx]
        mb_advantages = advantages_flat[mb_idx]
        mb_targets = target_values_flat[mb_idx]

        logits = self.actor(mb_states)
        mb_values = self.get_value(mb_states)

        (
          L_spo,
          S_pi,
          max_prob,
          approx_kl,
          clip_frac,
          spo_rest_frac,
          dc_frac,
          ess
        ) = compute_spo_dual_clip_loss(
          logits=logits,
          mb_actions=mb_actions,
          mb_old_logprobs=mb_old_logprobs,
          mb_advantages=mb_advantages,
          clip_eps_low=self.config.clip_eps_low,
          clip_eps_high=self.config.clip_eps_high,
          dual_clip_c=self.config.dual_clip_c,
          ratio_cap=self.config.spo_ratio_cap
        )

        if float(approx_kl.detach().item()) > self.config.target_kl:
          do_actor_step = False

        vf_scale = max(return_scale_std, 1e-4) if self.config.scale_vf else 1.0
        if self.config.clip_vloss:
          v_clipped = mb_old_values + (mb_values - mb_old_values).clamp(
            -self.config.clip_eps, +self.config.clip_eps
          )
          l_unclipped = ((mb_values - mb_targets) / vf_scale).pow(2)
          l_clipped = ((v_clipped - mb_targets) / vf_scale).pow(2)
          L_vf = 0.5 * torch.max(l_unclipped, l_clipped).mean()
        elif self.config.use_huber_vf:
          L_vf = F.smooth_l1_loss(
            mb_values / vf_scale, mb_targets / vf_scale, beta=1.0
          )
        else:
          L_vf = F.mse_loss(mb_values / vf_scale, mb_targets / vf_scale)

        if do_actor_step:
          loss = (
            L_spo
            + self.config.vf_coef * L_vf
            - self.config.ent_coef * S_pi
          )
          actor_batches += 1
        else:
          loss = self.config.vf_coef * L_vf

        self.optimizer.zero_grad(set_to_none=True)
        loss.backward()
        actor_gn = (
          nn.utils.clip_grad_norm_(
            self.actor.parameters(), max_norm=self.config.grad_clip
          ) if do_actor_step
          else torch.zeros((), device=self.device)
          )
        critic_gn = nn.utils.clip_grad_norm_(
          self.critic.parameters(), max_norm=self.config.grad_clip
        )
        self.optimizer.step()

        with torch.no_grad():
          L_vf_acc += L_vf.detach()
          S_pi_acc += S_pi.detach()
          approx_kl_acc += approx_kl.detach()
          clip_frac_acc += clip_frac.detach()
          spo_restoring_acc += spo_rest_frac.detach()
          dual_clip_acc += dc_frac.detach()
          ess_acc += ess.detach()
          max_prob_acc += max_prob.detach()
          actor_grad_acc += actor_gn.detach()
          critic_grad_acc += critic_gn.detach()
          total_batches += 1

    with torch.no_grad():
      actor_w_norm = torch.norm(
        torch.stack([p.norm(2) for p in self.actor.parameters()])
      )
      critic_w_norm = torch.norm(
        torch.stack([p.norm(2) for p in self.critic.parameters()])
      )

    denom = max(total_batches, 1)
    total_done = is_done.sum().clamp_min(1.0)
    mean_spo_loss = (L_spo_acc / total_batches).item()

    metrics = {
        "critic/explained_variance": explained_var.item(),
        "critic/value_loss": (L_vf_acc / denom).item(),
        "critic/value_mean": y_pred.mean().item(),
        "critic/value_std": y_pred.std(unbiased=False).item(),
        "critic/return_mean": y_true.mean().item(),
        "critic/return_std": y_true.std(unbiased=False).item(),
        "critic/td_residual_abs_mean": delta.abs().mean().item(),
        "critic/return_scale_std": float(return_scale_std),

        "policy/loss_spo": mean_spo_loss,
        "policy/loss_clip": mean_spo_loss,
        "policy/approx_kl": (approx_kl_acc / denom).item(),
        "policy/clip_fraction": (clip_frac_acc / denom).item(),
        "policy/spo_restoring_frac": (spo_restoring_acc / denom).item(),
        "policy/dual_clip_frac": (dual_clip_acc / denom).item(),
        "policy/ess": (ess_acc / denom).item(),
        "policy/entropy": ((S_pi_acc / denom) / self.entropy_norm_const).item(),
        "policy/max_action_prob": (max_prob_acc / denom).item(),
        "policy/advantage_raw_std": raw_adv_std.item(),
        "policy/actor_batches_frac": float(actor_batches) / float(denom),

        "grad/actor_grad_norm": (actor_grad_acc / denom).item(),
        "grad/critic_grad_norm": (critic_grad_acc / denom).item(),

        "plasticity/actor_weight_norm": actor_w_norm.item(),
        "plasticity/critic_weight_norm": critic_w_norm.item(),

        "actions/steer_left_frac": (steer_idx == 0).float().mean().item(),
        "actions/steer_straight_frac": (steer_idx == 1).float().mean().item(),
        "actions/steer_right_frac": (steer_idx == 2).float().mean().item(),
        "actions/thrust_0_frac": (thrust_idx == 0).float().mean().item(),
        "actions/thrust_200_frac": (thrust_idx == 1).float().mean().item(),
        "actions/boost_frac": (thrust_idx == 2).float().mean().item(),
        "actions/shield_frac": (thrust_idx == 3).float().mean().item(),
        "actions/same_action_frac": (pod0_act == pod1_act).float().mean().item(),

        "game/episode_completed": is_done.sum().item(),
        "game/win_rate": ((dones == 1).sum().float() / total_done).item(),
        "game/loss_rate": ((dones == 2).sum().float() / total_done).item(),
        "game/draw_rate": ((dones == 3).sum().float() / total_done).item(),
        "game/mean_episode_length": (
            (self.config.episode_steps * self.config.num_envs) / total_done
        ).item(),
    }

    return metrics

if __name__ == "__main__":
  config = SPOConfig()
  agent = SPOAgent(config)

  T, E = agent.batch_dim
  for i in range(T):
    state = torch.randn((E, config.state_dim))
    next_state = torch.randn_like(state)
    reward = torch.randn((E, 1))
    done = (torch.randn((E, 1)) < 0.05).long()

    agent.act(state)
    agent.observe(reward, next_state, done)

    if i % 10 == 0:
      agent.act(state, training=False)

  metrics = agent.update()
  print(f"{state.shape=} {agent.act(state, training=False).shape=}")
  print(f"Exp Var: {metrics['critic/explained_variance']:.4f}, SPO Loss{metrics['policy/loss_spo']:.4f}")
  print(f"Params: {agent.parameter_count()=}")
  print("Ok") 
