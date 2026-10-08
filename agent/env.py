#!/usr/bin/env python3
import torch
from engine import VectorEnv
import numpy as np

from state import State

RAW_STATE_DIM = 366
POD_NB = 4

class VecEnv:
  def __init__(self, num_envs, seed=42, device="cpu"):
    self.num_envs = num_envs
    self.device = device
    self.envs = VectorEnv(num_envs, seed)

    pin = (device != "cpu")

    self.state0_cpu = torch.zeros((1, num_envs, RAW_STATE_DIM), dtype=torch.float32, pin_memory=pin)
    self.state1_cpu = torch.zeros((1, num_envs, RAW_STATE_DIM), dtype=torch.float32, pin_memory=pin)
    self.rewards_cpu = torch.zeros((1, num_envs, 1), dtype=torch.float32, pin_memory=pin)
    self.dones_cpu = torch.zeros((1, num_envs, 1), dtype=torch.float32, pin_memory=pin)

    self._state0_np = self.state0_cpu.numpy()
    self._state1_np = self.state1_cpu.numpy()
    self._rewards_np = self.rewards_cpu.numpy()
    self._dones_np = self.dones_cpu.numpy()

  def reset(self) -> State:
    self.envs.reset(self._state0_np, self._state1_np)
    return State(self.state0_cpu.to(self.device, non_blocking=True)), \
           State(self.state1_cpu.to(self.device, non_blocking=True))

  def step(self, actions_tensor: torch.Tensor) -> State:
    actions_np = np.ascontiguousarray(actions_tensor.cpu().numpy())
    self.envs.step(actions_np, self._state0_np, self._state1_np, self._rewards_np, self._dones_np)

    state0_gpu = self.state0_cpu.to(self.device, non_blocking=True)
    state1_gpu = self.state1_cpu.to(self.device, non_blocking=True)
    rewards_gpu = self.rewards_cpu.to(self.device, non_blocking=True)
    dones_gpu = self.dones_cpu.to(self.device, non_blocking=True)

    return State(state0_gpu), State(state1_gpu), rewards_gpu, dones_gpu

if __name__ == "__main__":
  env = VecEnv(1)

  env.reset()
