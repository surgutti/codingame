#!/usr/bin/env python3

import torch
import time

from torch.utils.tensorboard import SummaryWriter
# from torch.profiler import profile, record_function, ProfilerActivity
from tqdm import tqdm

from arena import Arena, BotEntry
from config import PPOConfig
from env import VecEnv, RAW_STATE_DIM
from ppo import PPOAgent
from dummy import DummyAgent
from features import extract_features
from state import State
from stats import CUSTOM_LAYOUT

type Agent = PPOAgent | DummyAgent

def train(
  config: PPOConfig,
  writer: SummaryWriter,
  envs: VecEnv,
  agent0: PPOAgent,
  agent1: Agent,
  num_episodes: int,
  start_episode: int
):
  shape = [config.episode_steps, config.num_envs]
  states = torch.zeros((*shape, agent0.state_dim), dtype=torch.float32, device=config.device)
  actions = torch.zeros((*shape, 1), dtype=torch.long, device=config.device)
  rewards = torch.zeros((*shape, 1), dtype=torch.float32, device=config.device)
  dones = torch.zeros_like(rewards)
  logprobs = torch.zeros_like(rewards)
  values = torch.zeros_like(rewards)
  next_values = torch.zeros_like(rewards)
  
  state0_raw, state1_raw = envs.reset()

  total_wins = torch.zeros((), device=config.device)
  total_losses = torch.zeros((), device=config.device)
  total_draws = torch.zeros((), device=config.device)

  with tqdm(
    range(start_episode, start_episode + num_episodes),
    initial=start_episode,
    total=start_episode + num_episodes,
    desc="Training",
    unit="ep"
  ) as pbar:
    for episode in pbar:

      inference_time = 0.0
      simulation_time = 0.0
      update_time = 0.0

      rewards_acc = torch.zeros((), device=config.device)

      agent0.eval()

      for step in range(config.episode_steps):
        torch.cuda.synchronize()
        start = time.perf_counter()

        state0 = agent0.encode_state(state0_raw)
        state1 = agent1.encode_state(state1_raw)

        with torch.no_grad():
          action0, value, logprob = agent0.act_with_value_and_logprob(state0)
          action1 = agent1.act(state1)

        env_action0 = agent0.decode_action(action0)
        env_action1 = agent1.decode_action(action1)
        env_action = torch.cat([env_action0, env_action1], -1)

        torch.cuda.synchronize()
        inference_time += (time.perf_counter() - start)

        start = time.perf_counter()
        next_state0_raw, next_state1_raw, reward, done = envs.step(env_action)

        torch.cuda.synchronize()
        simulation_time += (time.perf_counter() - start)
        
        states[step] = state0
        actions[step] = action0
        rewards[step] = reward
        dones[step] = done
        logprobs[step] = logprob
        values[step] = value
        if step > 0:
          next_values[step - 1] = value

        state0_raw = next_state0_raw
        state1_raw = next_state1_raw

        rewards_acc += reward.mean()

      with torch.no_grad():
        next_values[-1] = agent0.get_value(agent0.encode_state(state0_raw))

      total_wins += (dones == 1.0).sum()
      total_losses += (dones == 2.0).sum()
      total_draws += (dones == 3.0).sum()
    
      torch.cuda.synchronize()
      start = time.perf_counter()

      agent0.train()
      metrics = agent0.update(
        states=states,
        actions=actions,
        rewards=rewards,
        dones=dones,
        logprobs=logprobs,
        values=values,
        next_values=next_values
      )

      for tag, val in metrics.items():
        writer.add_scalar(tag, val, episode)

      torch.cuda.synchronize()
      update_time += (time.perf_counter() - start)

      total_time = inference_time + simulation_time + update_time
      writer.add_scalar('perf/inference', inference_time, episode)
      writer.add_scalar('perf/simulation', simulation_time, episode)
      writer.add_scalar('perf/update', update_time, episode)
      writer.add_scalar('perf/sps', config.num_envs * config.episode_steps / total_time, episode)

      writer.add_scalar(
        'reward', (rewards_acc / config.episode_steps).item(), episode
      )
  
  return total_wins.item(), total_losses.item(), total_draws.item()

if __name__ == "__main__":
  config = PPOConfig()
  writer = SummaryWriter(log_dir=f"runs/ppo_{config.name}")
  writer.add_custom_scalars(CUSTOM_LAYOUT)
  envs = VecEnv(config.num_envs, config.seed, config.device)
 
  init_state = State(torch.ones((1, 1, RAW_STATE_DIM), dtype=torch.float32, device=config.device))
  state_dim = extract_features(init_state).shape[-1]

  agent0 = PPOAgent(config, state_dim).to(config.device)

  arena = Arena(config, state_dim, max_bots=config.max_bots)
  learner_entry = BotEntry(name="learner", path="learner", elo=1000.0)
  global_ep = 0

  for gen in range(config.total_episodes // config.episodes_per_gen):
    opp_entry, agent1 = arena.sample_opponent(learner_entry.elo)
    wins, losses, draws = train(
      config, writer, envs, agent0, agent1,
      num_episodes=config.episodes_per_gen, start_episode=global_ep
    )
    global_ep += config.episodes_per_gen

    arena.update_elo(learner_entry, opp_entry, wins, losses, draws)

    ckpt_entry = arena.register_and_prune(gen, agent0, envs, global_ep, start_elo=learner_entry.elo)
    learner_entry.elo = ckpt_entry.elo

    elos = [b.elo for b in arena.pool if b.path is not None]
    writer.add_scalar("arena/learner_elo", learner_entry.elo, global_ep)
    writer.add_scalar("arena/top1_elo", max(elos), global_ep)
    writer.add_scalar(f"arena/top{config.max_bots}_elo", min(elos), global_ep)

    md = "| Rank | Bot | Elo | Games |\n|---|---|---|---|\n"
    for idx, b in enumerate(sorted(arena.pool, key=lambda x: x.elo, reverse=True), 1):
      md += f"| {idx} | `{b.name}` | **{b.elo:.0f}** | {b.games} |\n"
    writer.add_text("arena/leaderboard", md, global_ep)

  writer.close() 

