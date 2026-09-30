#!/usr/bin/env python3
import torch
import torch.nn as nn
from torch.distributions.categorical import Categorical

from model import ActorNetwork, CriticNetwork
from config import SPOConfig

class SPOAgent(nn.Module):
  def __init__(
    self, 
    config: SPOConfig,
  ):
    super().__init__()
    self.config = config
    self.batch_dim = (config.episode_steps, config.num_envs)
    
    MAX_ROT = 0.3141592653589793
    angles = torch.tensor([[-MAX_ROT], [0.0], [+MAX_ROT]])
    thrusts = torch.tensor([
      [0.0,   0.0, 0.0],
      [200.0, 0.0, 0.0],
      [0.0,   0.0, 1.0],
      [0.0,   1.0, 0.0],
    ])

    pod_action = torch.cat([
      angles.repeat(4, 1),
      thrusts.repeat_interleave(3, dim=0)
    ], dim=1)

    self.action_list = torch.cat([
      pod_action.repeat_interleave(12, dim=0),
      pod_action.repeat(12, 1)
    ], dim=1).to(config.device)

    for i in range(self.action_list.shape[0]):
      print(i, self.action_list[i])

    state_dim  = config.state_dim
    action_dim = len(self.action_list)

    self.actor = torch.compile(ActorNetwork(action_dim))
    self.critic = torch.compile(CriticNetwork())

    self.optimizer = torch.optim.AdamW(
      [ 
        {'params': self.critic.parameters()},
        {'params': self.actor.parameters()}
      ],
      lr=config.learning_rate,
      eps=1e-5,
      fused=(config.device != "cpu")
    )

    self.batch_idx   = 0
    self.states      = torch.zeros((*self.batch_dim, state_dim), dtype=torch.float32, device=config.device)
    self.actions     = torch.zeros((*self.batch_dim,         1), dtype=torch.long   , device=config.device)
    self.rewards     = torch.zeros((*self.batch_dim,         1), dtype=torch.float32, device=config.device)
    self.dones       = torch.zeros((*self.batch_dim,         1), dtype=torch.long   , device=config.device)
    self.logprobs    = torch.zeros((*self.batch_dim,         1), dtype=torch.float32, device=config.device)
    self.values      = torch.zeros((*self.batch_dim,         1), dtype=torch.float32, device=config.device)
    self.next_values = torch.zeros((*self.batch_dim,         1), dtype=torch.float32, device=config.device)

  def get_value(self, state: torch.Tensor) -> torch.Tensor:
    return self.critic(state)

  def observe(self, reward, next_state, done) -> torch.Tensor:
    with torch.no_grad():
      self.rewards[self.batch_idx] = reward
      self.next_values[self.batch_idx] = self.get_value(next_state)
      self.dones[self.batch_idx] = done

    self.batch_idx += 1

  def act_dist(self, state: torch.Tensor) -> Categorical:
    return Categorical(logits=self.actor(state), validate_args=False)
    
  @torch.no_grad()
  def act(self, state: torch.Tensor, is_training: bool = True) -> torch.Tensor:
    dist = self.act_dist(state)
    action_idx = dist.sample()
    action = self.action_list[action_idx]

    if is_training:
      self.states[self.batch_idx] = state
      self.actions[self.batch_idx] = action_idx.unsqueeze(-1)
      self.logprobs[self.batch_idx] = dist.log_prob(action_idx).unsqueeze(-1)
      self.values[self.batch_idx] = self.get_value(state)

    return action

  def parameter_count(self):
    return sum(p.numel() for p in self.parameters())
  
  def update(self):
    T, E = self.batch_dim
    B = T * E

    assert self.batch_idx == B
    self.batch_idx = 0

    is_done = (self.dones > 0.5).float()
    delta = self.rewards + self.config.gamma * self.next_values * (1.0 - is_done) - self.values

    advantages = torch.zeros_like(delta)
    A = torch.zeros_like(self.values[0]) # (E, 1)

    is_done_bool = is_done.bool()
    for i in range(T - 1, -1, -1):
      A = torch.where(is_done_bool[i], 0.0, A)
      A = delta[i] + self.config.gamma * self.config.gae_lambda * A
      advantages[i] = A

    target_values = self.values + advantages
    raw_adv_std = advantages.std()

    y_pred = self.values.flatten()
    y_true = target_values.flatten()
    var_y = torch.var(y_true)
    explained_var = 1.0 - torch.var(y_true - y_pred) / (var_y + 1e-8)

    act_flat = self.actions.flatten()
    pod0_act, pod1_act = act_flat // 12, act_flat % 12
    all_pods_acts = torch.cat([pod0_act, pod1_act], dim=0)
    steer_idx = all_pods_acts % 3
    thrust_idx = all_pods_acts // 3

    if self.config.norm_adv:
      advantages = (advantages - advantages.mean()) / (advantages.std() + 1e-8)
   
    states        = self.states.flatten(0, 1)
    actions       = self.actions.flatten(0, 1)
    rewards       = self.rewards.flatten(0, 1)
    dones         = self.dones.flatten(0, 1)
    logprobs      = self.logprobs.flatten(0, 1)
    values        = self.values.flatten(0, 1)
    next_values   = self.next_values.flatten(0, 1)
    target_values = self.target_values.flatten(0, 1)
    advantages    = advantages.flatten(0, 1)

    assert actions.shape == (B, 1)

    L_clip_acc = torch.zeros((), device=states.device)
    L_vf_acc = torch.zeros((), device=states.device)
    S_pi_acc = torch.zeros((), device=states.device)
    approx_kl_acc = torch.zeros((), device=states.device)
    clip_frac_acc = torch.zeros((), device=states.device)
    max_prob_acc = torch.zeros((), device=states.device)
    actor_grad_acc = torch.zeros((), device=states.device)
    critic_grad_acc = torch.zeros((), device=states.device)
    total_batches = 0

    for epoch in range(self.config.update_epochs):
      inds = torch.randperm(B, device=self.config.device)

      for start in range(0, B, self.config.minibatch_size):
        end = min(start + self.config.minibatch_size, B)
        mb_idx = inds[start:end]

        mb_states = states[mb_idx]
        mb_actions = actions[mb_idx]
        mb_rewards = rewards[mb_idx]
        mb_old_logprobs = logprobs[mb_idx]
        mb_advantages = advantages[mb_idx]
        mb_targets = target_values[mb_idx]

        dist = self.act_dist(mb_states)
        mb_logprobs = dist.log_prob(mb_actions.squeeze(-1)).unsqueeze(-1)
        mb_values = self.get_value(mb_states)
      
        log_ratio = mb_logprobs - mb_old_logprobs
        ratio = torch.exp(log_ratio)

        surr1 = ratio * mb_advantages
        #surr2 = ratio.clamp(
        #          1.0 - self.config.clip_eps, 
        #          1.0 + self.config.clip_eps
        #        ) * mb_advantages
        surr3 = mb_advantages.abs() * (0.5 * 1/self.clip_eps) * (ratio - 1).pow(2)

        L_spo = - (surr1 - surr3)
        #L_clip = -torch.min(surr1, surr2).mean()
        L_vf = nn.functional.mse_loss(mb_values, mb_targets)
        S_pi = dist.entropy().mean()

        loss = L_spo + self.config.vf_coef * L_vf - self.config.ent_coef * S_pi

        self.optimizer.zero_grad()
        loss.backward()
        actor_gn = nn.utils.clip_grad_norm_(self.actor.parameters(), max_norm=self.config.grad_clip)
        critic_gn = nn.utils.clip_grad_norm_(self.critic.parameters(), max_norm=self.config.grad_clip)
        self.optimizer.step()

        with torch.no_grad():
          L_clip_acc += L_clip.detach()
          L_vf_acc += L_vf.detach()
          S_pi_acc += S_pi.detach()
          approx_kl_acc += ((ratio - 1.0) - log_ratio).mean()
          clip_frac_acc += ((ratio - 1.0).abs() > self.config.clip_eps).float().mean()
          max_prob_acc += dist.probs.max(dim=-1).values.mean()
          actor_grad_acc += actor_gn
          critic_grad_acc += critic_gn
          total_batches += 1
    
    with torch.no_grad():
      actor_w_norm = torch.norm(torch.stack([p.norm(2) for p in self.actor.parameters()]))
      critic_w_norm = torch.norm(torch.stack([p.norm(2) for p in self.critic.parameters()]))

    metrics = {
      "critic/explained_variance": explained_var.item(),
      "critic/value_loss": (L_vf_acc / total_batches).item(),
      "critic/value_mean": y_pred.mean().item(),
      "critic/value_std": y_pred.std().item(),
      "critic/return_mean": y_true.mean().item(),
      "critic/return_std": y_true.std().item(),
      "critic/td_residual_abs_mean": delta.abs().mean().item(),

      "policy/loss_clip": (L_clip_acc / total_batches).item(),
      "policy/approx_kl": (approx_kl_acc / total_batches).item(),
      "policy/clip_fraction": (clip_frac_acc / total_batches).item(),
      "policy/entropy": ((S_pi_acc / total_batches) / 4.394449).item(),
      "policy/max_action_prob": (max_prob_acc / total_batches).item(),
      "policy/advantage_raw_std": raw_adv_std.item(),
      
      "grad/actor_grad_norm": (actor_grad_acc / total_batches).item(),
      "grad/critic_grad_norm": (critic_grad_acc / total_batches).item(),

      "plasticity/actor_weight_norm": actor_w_norm.item(),
      "plasticity/critic_weight_norm": critic_w_norm.item(),
      
      "actions/steer_left_frac": (steer_idx == 0).float().mean().item(),
      "actions/steer_straight_frac": (steer_idx == 1).float().mean().item(),
      "actions/steer_right_frac": (steer_idx == 2).float().mean().item(),
      "actions/thrust_0_frac": (thrust_idx == 0).float().mean().item(),
      "actions/thrust_200_frac": (thrust_idx == 1).float().mean().item(),
      "actions/shield_frac": (thrust_idx == 2).float().mean().item(),
      "actions/same_action_frac": (pod0_act == pod1_act).float().mean().item(),

      "game/episode_completed": is_done.sum().item(),
      "game/win_rate": ((dones == 1).sum().float() / is_done.sum().clamp_min(1)).item(),
      "game/loss_rate": ((dones == 2).sum().float() / is_done.sum().clamp_min(1)).item(),
      "game/draw_rate": ((dones == 3).sum().float() / is_done.sum().clamp_min(1)).item(),
      "game/mean_episode_length": ((self.config.episode_steps * self.config.num_envs) / is_done.sum().clamp_min(1)).item()
    }

    return metrics

if __name__ == "__main__":

  config = SPOConfig()

  agent = SPOAgent(config)

  B, E = agent.batch_dim

  for i in range(B):
    state = torch.randn((E, config.state_dim))
    next_state = torch.randn_like(state)
    reward = torch.randn((E, 1))
    done = torch.randn((E, 1), dtype=torch.float32).long()

    agent.act(state)
    agent.observe(next_state, reward, done)
    
    if i % 100 == 0:
      agent.act(state, training=False)
    
  agent.update()
  print(f"{state.shape}")

  print(agent.act(state))
  print("Ok") 
