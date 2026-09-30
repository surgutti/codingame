#!/usr/bin/env python3

import torch
import torch.nn as nn
import torch.nn.functional as F

def layer_init(layer, std=2**0.5, bias_const=0.0):
  nn.init.orthogonal_(layer.weight, std)
  nn.init.constant_(layer.bias, bias_const)
  return layer

class SwiGLU(nn.Module):
  def __init__(
    self,
    input_dim: int,
    hidden_dim: int,
    output_dim: int
  ):
    super().__init__()

    self.W = layer_init(nn.Linear(input_dim, hidden_dim))
    self.V = layer_init(nn.Linear(input_dim, hidden_dim))
    self.W2 = layer_init(nn.Linear(hidden_dim, output_dim))
    self.beta = nn.Parameter(torch.ones(()))

  def forward(self, x):
    return self.W2(F.silu(self.W(x) * self.beta) * self.V(x)) 

class StateEmbedding(nn.Module):
  def __init__(
    self,
    output_pods,
    output_cps,
    output_meta
  ):
    super().__init__()

    self.pod_embd = SwiGLU(8, int(output_pods * 1.5), output_pods)
    self.cp_embd = SwiGLU(2, int(output_cps * 1.5), output_cps)
    self.meta_embd = nn.Linear(4, output_meta)

    self.output_dim = 4 * output_pods + 6 * output_cps + 1 * output_meta

  def forward(self, state: torch.Tensor) -> torch.Tensor:
    assert state.shape[-1] == 48

    pods = state[..., :32].unflatten(-1, (4, 8))
    cps = state[..., 32:44].unflatten(-1, (6, 2))
    meta = state[..., 44:48]

    return torch.cat([
      self.pod_embd(pods).flatten(-2),
      self.cp_embd(cps).flatten(-2),
      self.meta_embd(meta)
    ], dim=-1)

class ActorNetwork(nn.Module):
  def __init__(
    self,
    action_dim: int
  ):
    super().__init__()

    self.embd = StateEmbedding(
      output_pods=16,
      output_cps=8,
      output_meta=8
    )

    self.core = nn.Sequential(
      SwiGLU(self.embd.output_dim, 128, 128),
      nn.LayerNorm(128),
      nn.SiLU(),
      layer_init(nn.Linear(128, action_dim), std=0.01)
    )

  def forward(self, state):
    return self.core(self.embd(state))

class CriticNetwork(nn.Module):
  def __init__(
    self,
  ):
    super().__init__()

    self.embd = StateEmbedding(
      output_pods=128,
      output_cps=64,
      output_meta=16
    )

    self.core = nn.Sequential(
      SwiGLU(self.embd.output_dim, 256, 128),
      nn.LayerNorm(128),
      nn.SiLU(),
      layer_init(nn.Linear(128, 1), std=1.0)
    )

  def forward(self, state):
    return self.core(self.embd(state))

if __name__ == "__main__":

  actor = ActorNetwork(81)
  critic = CriticNetwork()
  state = torch.randn((21, 37, 67, 48))
  
  assert actor(state).shape == (21, 37, 67, 81)
  assert critic(state).shape == (21, 37, 67, 1)

  print("Actor params: ", sum(p.numel() for p in actor.parameters()))
  print("Critic params: ", sum(p.numel() for p in critic.parameters()))

  print("Ok")
