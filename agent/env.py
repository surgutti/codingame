import torch
from engine import VectorEnv
import numpy as np

RAW_STATE_DIM = 48
POD_NB = 4

class VecEnv:
  def __init__(self, num_envs, seed=42, device="cuda"):
    self.num_envs = num_envs
    self.device = device
    self.envs = VectorEnv(num_envs, seed)

    self.state_cpu = torch.zeros((num_envs, RAW_STATE_DIM), dtype=torch.float32, pin_memory=True)
    self.rewards_cpu = torch.zeros((num_envs,), dtype=torch.float32, pin_memory=True)
    self.dones_cpu = torch.zeros((num_envs,), dtype=torch.float32, pin_memory=True)

    self._state_np = self.state_cpu.numpy()
    self._rewards_np = self.rewards_cpu.numpy()
    self._dones_np = self.dones_cpu.numpy()

  def reset(self):
    self.envs.reset(self._state_np)
    return self.state_cpu.to(self.device, non_blocking=True)

  def step(self, actions_tensor: torch.Tensor):
    actions_np = np.ascontiguousarray(actions_tensor.cpu().numpy())
    self.envs.step(actions_np, self._state_np, self._rewards_np, self._dones_np)

    state_gpu = self.state_cpu.to(self.device, non_blocking=True)
    rewards_gpu = self.rewards_cpu.to(self.device, non_blocking=True)
    dones_gpu = self.dones_cpu.to(self.device, non_blocking=True)

    return state_gpu, rewards_gpu, dones_gpu
