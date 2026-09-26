import torch import torch.nn as nn
from torch.distributions.categorical import Categorical
from state import State, get_checkpoint_xy

from config import PPOConfig
from features import extract_features

def layer_init(layer, std=np.sqrt(2), bias_const=0.0):
  torch.nn.init.orthogonal_(layer.weight, std)
  torch.nn.init.constant_(layer.bias, bias_const)
  return layer

STATE_DIM = ?
ACTION_DIM = 9

class PPOAgent(nn.Module):
  def __init__(
    self, 
    config: PPOConfig,
  ):
    super().__init__()
    self.config = config

    self.critic = nn.Sequential(
      layer_init(nn.Linear(STATE_DIM, 256)),
      nn.LayerNorm(256),
      nn.SiLU(),
      layer_init(nn.Linear(256, 256)),
      nn.LayerNorm(256),
      nn.SiLU(),
      layer_init(nn.Linear(256, 1), std=1.0)
    )

    self.actor = nn.Sequential(
      layer_init(nn.Linear(STATE_DIM, 128)),
      nn.LayerNorm(128),
      nn.SiLU(),
      layer_init(nn.Linear(128, 128)),
      nn.LayerNorm(128),
      nn.SiLU(),
      layer_init(nn.Linear(128, ACTION_DIM), std=0.01)
    )

  def get_value(self, state: torch.Tensor) -> torch.Tensor
    return self.critic(state).squeeze(-1)

  def act_dist(self, state: torch.Tensor) -> torch.Tensor:
    return Categorical(logits=self.actor(state))

  def act(self, state: torch.Tensor) -> torch.Tensor:
    return self.act_dist(state).sample().squeeze(-1)

  def act_with_value_and_logprob(self, state: torch.Tensor):
    dist = self.act_dist(state)
   
    action = dist.sample().squeeze(-1)
    value = self.get_value(state).squeeze(-1)
    logprob = dist.log_prob(action)
    
    return action, value, logprob

  def encode_state(self, state: State) -> torch.Tensor:
    return extract_features(state)

  def decode_action(
    self, 
    action: torch.Tensor # [B, E, 1]
  ) -> torch.Tensor:
    MAX_ROT = 0.3141592653589793
    ACTIONS = torch.tensor([
      [-MAX_ROT,   0.0, 0.0, 0.0], [0.0,   0.0, 0.0, 0.0], [+MAX_ROT,   0.0, 0.0, 0.0],
      [-MAX_ROT, 200.0, 0.0, 0.0], [0.0, 200.0, 0.0, 0.0], [+MAX_ROT, 200.0, 0.0, 0.0],
      [-MAX_ROT,   0.0, 1.0, 0.0], [0.0,   0.0, 1.0, 0.0], [+MAX_ROT,   0.0, 1.0, 0.0],
    ], dtype=torch.float32)

    d_action = ACTIONS[action].squeeze(-2)
    return d_action

  def update(
    self,
    states,      # (T, E, state_dim)
    actions,     # (T, E, action_dim)
    rewards,     # (T, E, 1)
    dones,       # (T, E, 1)
    next_states, # (T, E, state_dim)
    logprobs,    # (T, E, 1)
    values,      # (T, E, 1)
    next_values  # (T, E, 1)
  ):
    T = dones.shape[0]

    target_values = rewards + self.config.gamma * next_values * (1.0 - dones)
    delta = target_valeus - values

    advantages = torch.zeros_like(delta)
    A = torch.zeros_like(values[0]) # (E, 1)
    for i in reversed(range(T)):
      A = torch.where(dones[i], 0.0, A)
      A = delta[i] + self.config.gamma * self.config.gae_lambda * A
      advantages[i] = A

    advantages = (advantages - advantages.mean()) / (advantages.std() + 1e-8)

    L_clip_acc = 0
    L_vf_acc = 0
    S_pi_acc = 0
    total_batches = 0

    for epoch in range(self.config.update_epochs):
      inds = torch.randperm(T, device=self.config.device)

      for start in range(0, T, self.config.minibatch_size):
        end = min(start + self.config.minibatch_size, T)
        
        mb_idx = inds[start:end]

        mb_states = states[mb_idx]
        mb_actions = actions[mb_idx]
        mb_rewards = rewards[mb_idx]
        mb_old_logprobs = logprobs[mb_idx]
        mb_advantages = advantages[mb_idx]
        mb_targets = target_values[mb_idx]

        dist = self.act_dist(mb_states)
        logprobs = dist.log_probs(mb_actions)
        values = self.get_value(mb_states)
      
        ratio = torch.exp(logprobs - mb_old_logprobs)

        surr1 = ratio * mb_advantages
        surr2 = ratio.clamp(
                  1.0 - self.config.clip_eps, 
                  1.0 + self.config.clip_eps
                ) * mb_advantages

        L_clip = -torch.min(surr1, surr2).mean()
        L_vf = nn.functional.mse_loss(values, mb_targets)
        S_pi = dist.entropy().mean()

        loss = L_clip + self.config.vf_coef * L_vf - self.config.ent_coef * S_pi

        self.optimizer.zero_grad()
        loss.backward()
        nn.utils.clip_grad_norm_(self.net.parameters(), max_norm=self.config.grad_clipping)
        self.optimizer.step()

        L_clip_acc += L_clip.item()
        L_vf_acc += L_vf.item()
        S_pi_acc += S_pi.item()

        total_batches += 1
      
      return L_clip_acc / total_batches, \
             L_vf_acc / total_batches, \
             S_pi_acc / total_batches
      
