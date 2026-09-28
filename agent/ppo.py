import torch
import torch.nn as nn
from torch.distributions.categorical import Categorical
from state import State, get_checkpoint_xy

from config import PPOConfig
from features import extract_features

def layer_init(layer, std=2**0.5, bias_const=0.0):
  torch.nn.init.orthogonal_(layer.weight, std)
  torch.nn.init.constant_(layer.bias, bias_const)
  return layer

ACTION_DIM = 81

class PPOAgent(nn.Module):
  def __init__(
    self, 
    config: PPOConfig,
    state_dim: int,
  ):
    super().__init__()
    self.config = config
    self.action_dim = ACTION_DIM
    self.state_dim = state_dim

    self.critic = torch.compile(nn.Sequential(
      layer_init(nn.Linear(state_dim, 256)),
      nn.LayerNorm(256),
      nn.SiLU(),
      layer_init(nn.Linear(256, 256)),
      nn.LayerNorm(256),
      nn.SiLU(),
      layer_init(nn.Linear(256, 1), std=1.0)
    ))

    self.actor = torch.compile(nn.Sequential(
      layer_init(nn.Linear(state_dim, 128)),
      nn.LayerNorm(128),
      nn.SiLU(),
      layer_init(nn.Linear(128, 128)),
      nn.LayerNorm(128),
      nn.SiLU(),
      layer_init(nn.Linear(128, ACTION_DIM), std=0.01)
    ))

    self.optimizer = torch.optim.AdamW(
      [ 
        {'params': self.critic.parameters()},
        {'params': self.actor.parameters()}
      ],
      lr=config.learning_rate,
      eps=1e-5,
      fused=(config.device != "cpu")
    )

    MAX_ROT = 0.3141592653589793

    ANGLE = torch.tensor([[-MAX_ROT], [0.0], [+MAX_ROT]])
    THRUST = torch.tensor([
      [0.0,   0.0, 0.0],
      [200.0, 0.0, 0.0],
      [0.0,   1.0, 0.0]
    ])

    ACTIONS = torch.cat([
      ANGLE.repeat(3, 1),
      THRUST.repeat_interleave(3, dim=0)
    ], dim=1) # [9, 4]

    self.ACTIONS_2 = torch.cat([
      ACTIONS.repeat_interleave(9, dim=0),
      ACTIONS.repeat(9, 1)
    ], dim=1).to(config.device) # [81, 8]

  def get_value(self, state: torch.Tensor) -> torch.Tensor:
    return self.critic(state)

  def act_dist(self, state: torch.Tensor) -> torch.Tensor:
    return Categorical(logits=self.actor(state), validate_args=False)

  def act(self, state: torch.Tensor) -> torch.Tensor:
    return self.act_dist(state).sample().unsqueeze(-1)

  def act_with_value_and_logprob(self, state: torch.Tensor):
    dist = self.act_dist(state)
   
    action = dist.sample()
    value = self.get_value(state)
    logprob = dist.log_prob(action)
    
    return action.unsqueeze(-1), value, logprob.unsqueeze(-1)

  def encode_state(self, state: State) -> torch.Tensor:
    return extract_features(state)

  def decode_action(
    self, 
    action: torch.Tensor # [B, E, 1]
  ) -> torch.Tensor:
    d_action = self.ACTIONS_2[action].squeeze(-2)
    return d_action

  def update(
    self,
    states,      # (T, E, state_dim)
    actions,     # (T, E, action_dim)
    rewards,     # (T, E, 1)
    dones,       # (T, E, 1)
    # next_states, # (T, E, state_dim)
    logprobs,    # (T, E, 1)
    values,      # (T, E, 1)
    next_values  # (T, E, 1)
  ):
    T = dones.shape[0]
    E = dones.shape[1]
    B = T * E

    delta = rewards + self.config.gamma * next_values * (1.0 - dones) - values

    advantages = torch.zeros_like(delta)
    A = torch.zeros_like(values[0]) # (E, 1)
    dones = dones.bool()

    for i in range(T - 1, -1, -1):
      A = torch.where(dones[i], 0.0, A)
      A = delta[i] + self.config.gamma * self.config.gae_lambda * A
      advantages[i] = A

    target_values = values + advantages
    raw_adv_std = advantages.std()

    y_pred = values.flatten()
    y_true = target_values.flatten()
    var_y = torch.var(y_true)
    explained_var = 1.0 - torch.var(y_true - y_pred) / (var_y + 1e-8)

    act_flat = actions.long().flatten()
    pod0_act, pod1_act = act_flat // 9, act_flat % 9
    all_pods_acts = torch.cat([pod0_act, pod1_act], dim=0)
    steer_idx = all_pods_acts % 3
    thrust_idx = all_pods_acts // 3

    if self.config.norm_adv:
      advantages = (advantages - advantages.mean()) / (advantages.std() + 1e-8)
   
    states = states.flatten(0, 1)
    actions = actions.flatten(0, 1)
    rewards = rewards.flatten(0, 1)
    dones = dones.flatten(0, 1)
    # next_states = next_states.flatten(0, 1)
    logprobs = logprobs.flatten(0, 1)
    values = values.flatten(0, 1)
    next_values = next_values.flatten(0, 1)

    target_values = target_values.flatten(0, 1)
    advantages = advantages.flatten(0, 1)

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
        surr2 = ratio.clamp(
                  1.0 - self.config.clip_eps, 
                  1.0 + self.config.clip_eps
                ) * mb_advantages

        L_clip = -torch.min(surr1, surr2).mean()
        L_vf = nn.functional.mse_loss(mb_values, mb_targets)
        S_pi = dist.entropy().mean()

        loss = L_clip + self.config.vf_coef * L_vf - self.config.ent_coef * S_pi

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
      "policy/entropy_raw": (S_pi_acc / total_batches).item(),
      "policy/entropy_normalized": ((S_pi_acc / total_batches) / 4.394449).item(),
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
    }

    return metrics
    
