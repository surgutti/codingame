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

    self.W = layer_init(nn.Linear(input_dim, hidden_dim), std=1.0)
    self.V = layer_init(nn.Linear(input_dim, hidden_dim), std=1.0)
    self.W2 = layer_init(nn.Linear(hidden_dim, output_dim))
    self.beta = nn.Parameter(torch.ones(()))

  def forward(self, x):
    return self.W2(F.silu(self.W(x) * self.beta) * self.V(x)) 

class ActorNetwork(nn.Module):
  def __init__(
    self,
    state_dim: int,
    action_dim: int,
  ):
    super().__init__()

    self.net = nn.Sequential(
      layer_init(nn.Linear(state_dim, 256)),
      nn.LayerNorm(256),
      nn.SiLU(),
      SwiGLU(256, 512, 256),
      nn.LayerNorm(256),
      nn.SiLU(),
      layer_init(nn.Linear(256, action_dim), std=0.01)
    )

  def forward(self, state):
    return self.net(state)

class CriticNetwork(nn.Module):
  def __init__(
    self,
    state_dim: int,
  ):
    super().__init__()
 
    self.net = nn.Sequential(
      layer_init(nn.Linear(state_dim, 256)),
      nn.LayerNorm(256),
      nn.SiLU(),
      SwiGLU(256, 512, 256),
      nn.LayerNorm(256),
      nn.SiLU(),
      layer_init(nn.Linear(256, 1), std=1.0)
    )

  def forward(self, state):
    return self.net(state)

if __name__ == "__main__":

  actor = ActorNetwork(81)
  critic = CriticNetwork()
  state = torch.randn((21, 37, 67, 48))
  
  print(actor.scale)
  assert actor(state).shape == (21, 37, 67, 81)
  assert critic(state).shape == (21, 37, 67, 1)

  print("Actor params: ", sum(p.numel() for p in actor.parameters()))
  print("Critic params: ", sum(p.numel() for p in critic.parameters()))

  print("Ok")
