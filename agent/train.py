#!/usr/bin/env python3

import torch
import time

from torch.utils.tensorboard import SummaryWriter

from env import VecEnv, RAW_STATE_DIM
from config import PPOConfig
from ppo import PPOAgent
from dummy import DummyAgent
from features import extract_features
from state import State

type Agent = PPOAgent | DummyAgent

def combine_actions(action0: torch.Tensor, action1: torch.Tensor) -> torch.Tensor:
  raise NotImplemented()

def train(
  config: PPOConfig(),
  writer: SummaryWriter,
  envs: VecEnv,
  agent0: PPOAgent,
  agent1: Agent):

  states = torch.zeros((config.episode_steps, state_dim), dtype=torch.float32, device=config.device)
  actions = torch.zeros((config.episode_steps, action_dim), dtype=torch.float32, device=config.device)
  rewards = torch.zeros((config.episode_steps, 1), dtype=torch.float32, device=config.device)
  dones = torch.zeros((config.episode_steps, 1), dtype=torch.float32, device=config.device)
  next_states = torch.zeros((config.episode_steps, state_dim), dtype=torch.float32, device=config.device)
  logprobs = torch.zeros((config.episode_steps, 1), dtype=torch.float32, device=config.device)
  values = torch.zeros((config.episode_steps, 1), dtype=torch.float32, device=config.device)
  
  state = envs.reset()

  for episode in tqdm(range(config.total_episodes)):

    inference_time = 0.0
    simulation_time = 0.0
    updating_time = 0.0

    for step in range(config.episode_steps):
      torch.cuda.synchronize()
      start = time.perf_counter()

      state0 = agent0.encode_state(state)
      state1 = agent1.encode_state(state.flip_teams())

      action0, value, logprob = agent.act_with_value_and_logprob(state0)
      action1 = agent.act(state1)

      env_action0 = agent0.decode_action(action0)
      env_action1 = agent1.decode_action(action1)

      env_action = torch.cat([env_action0, env_action1], -1)

      torch.cuda.synchronize()
      inference_time += (time.perf_counter() - start)

      start = time.perf_counter()
      next_state, reward, done = env.step(env_action)

      torch.cuda.syncronize()
      simulation_time += (time.perf_counter() - start)

      states[step] = state
      actions[step] = action0
      rewards[step] = reward
      dones[step] = done
      next_states[step] = next_state
      logprobs[step] = logprob
      values[step] = value

      state = next_state
    
    torch.cuda.synchronize()
    start = time.perf_counter()
    L_clip, L_vf, S_pi = agent0.update(
      states=states,
      actions=actions,
      rewards=rewards,
      dones=dones,
      next_states=next_states,
      logprobs=logprobs,
      values=values
    )

    torch.cuda.synchronize()
    updating_time += (time.perf_counter() - start)

    writer.add_scalar('L_clip', L_clip, episode)
    writer.add_scalar('L_vf', L_vf, episode)
    writer.add_scalar('S_pi', S_pi, episode)

    writer.add_scalars('Performance', { 
        'Inference': inference_time,
        'Simulation': simulation_time,
        'Updating': updating_time 
      }, episode
    )
  
  writer.close() 

if __name__ == "__main__":
  config = PPOConfig()
  writer = SummaryWriter(log_dir=f"runs/ppo_{config.name}")
  envs = VecEnv(config.num_envs, config.seed, config.device)
 
  init_state = State(torch.ones((1, RAW_STATE_DIM), dtype=torch.float32))  
  state_dim = extract_features(init_state).shape[0]

  print(f"State Dim: {state_dim}")
  print(f"Action Dim: {action_dim}")
  exit(0)

  agent0 = PPOAgent(config)
  agent1 = DummyAgent()

  train(agent0, agent1)
