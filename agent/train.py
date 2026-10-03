#!/usr/bin/env python3

import torch
import time

from torch.utils.tensorboard import SummaryWriter
# from torch.profiler import profile, record_function, ProfilerActivity
from tqdm import tqdm

from arena import Arena, BotEntry
from env import VecEnv, RAW_STATE_DIM
from spo.config import SPOConfig
from spo.spo import SPOAgent
from handmade.handmade import HandmadeAgent
from state import State
from stats import CUSTOM_LAYOUT

type Agent = PPOAgent | DummyAgent

def train(
  config: SPOConfig,
  writer: SummaryWriter,
  envs: VecEnv,
  agent0: Agent,
  arena: Arena,
  learner_entry: BotEntry,
  num_episodes: int,
  start_episode: int
):
  state0, state1 = envs.reset()

  with tqdm(
    range(start_episode, start_episode + num_episodes),
    initial=start_episode,
    total=start_episode + num_episodes,
    desc="Training",
    unit="ep"
  ) as pbar:
    for episode in pbar:
      opp_entry, agent1 = arena.sample_opponent(learner_entry.elo)

      inference_time = 0.0
      simulation_time = 0.0
      update_time = 0.0

      rewards_acc = torch.zeros((), device=config.device)

      agent0.eval()
      agent1.eval()

      ep_wins = torch.zeros((), device=config.device)
      ep_losses = torch.zeros((), device=config.device)
      ep_draws = torch.zeros((), device=config.device)
      for step in range(config.episode_steps):
        # torch.cuda.synchronize()
        start = time.perf_counter()

        with torch.no_grad():
          action0 = agent0.act(state0, training=True)
          action1 = agent1.act(state1)
    
        # print(f"{action0.shape=} {action1.shape=}")
        env_action = torch.cat([action0, action1], -1)

        # torch.cuda.synchronize()
        inference_time += (time.perf_counter() - start)

        start = time.perf_counter()
        next_state0, next_state1, reward, done = envs.step(env_action)

        agent0.observe(+reward, next_state0, done)
        # agent1.observe(-reward, next_state1, done)

        # torch.cuda.synchronize()
        simulation_time += (time.perf_counter() - start)

        state0 = next_state0
        state1 = next_state1

        rewards_acc += reward.mean()
        
        ep_wins += (done == 1.0).sum()
        ep_losses += (done == 2.0).sum()
        ep_draws += (done == 3.0).sum()

      arena.update_elo(
        learner_entry, 
        opp_entry, 
        ep_wins.item(), 
        ep_losses.item(), 
        ep_draws.item()
      )

      # torch.cuda.synchronize()
      start = time.perf_counter()

      agent0.train()
      metrics = agent0.update()

      for tag, val in metrics.items():
        writer.add_scalar(tag, val, episode)

      # torch.cuda.synchronize()
      update_time += (time.perf_counter() - start)

      total_time = inference_time + simulation_time + update_time
      writer.add_scalar('perf/inference', inference_time, episode)
      writer.add_scalar('perf/simulation', simulation_time, episode)
      writer.add_scalar('perf/update', update_time, episode)
      writer.add_scalar('perf/sps', config.num_envs * config.episode_steps / total_time, episode)

      writer.add_scalar(
        'reward', (rewards_acc / config.episode_steps).item(), episode
      )

if __name__ == "__main__":
  config = SPOConfig()
  envs = VecEnv(config.num_envs, config.seed, config.device)
 
  agent0 = SPOAgent(config).to(config.device)

  arena = Arena(config, max_bots=config.max_bots)
  learner_entry = BotEntry(name="learner", path="learner", elo=1000.0)
  global_ep = 0

  start_gen, global_ep = arena.load_state(agent0, learner_entry)

  writer = SummaryWriter(
    log_dir=f"runs/ppo_{config.name}",
    purge_step=global_ep if global_ep > 0 else None
  )
  if global_ep == 0:
    writer.add_custom_scalars(CUSTOM_LAYOUT)

  total_gens = config.total_episodes // config.episodes_per_gen

  try:
    for gen in range(start_gen, total_gens):
      train(
        config, 
        writer, 
        envs, 
        agent0,
        arena,
        learner_entry,
        num_episodes=config.episodes_per_gen, 
        start_episode=global_ep
      )
      global_ep += config.episodes_per_gen

      ckpt_entry = arena.register_and_prune(
        gen, agent0, envs, global_ep, start_elo=learner_entry.elo
      )
      learner_entry.elo = ckpt_entry.elo

      arena.save_state(agent0, learner_entry, next_gen=gen + 1, global_ep=global_ep)

      elos = [b.elo for b in arena.pool if b.path is not None]
      writer.add_scalar("arena/learner_elo", learner_entry.elo, global_ep)
      writer.add_scalar("arena/top1_elo", max(elos), global_ep)
      writer.add_scalar(f"arena/top{config.max_bots}_elo", min(elos), global_ep)

      md = "| Rank | Bot | Elo | Games |\n|---|---|---|---|\n"
      for idx, b in enumerate(sorted(arena.pool, key=lambda x: x.elo, reverse=True), 1):
        md += f"| {idx} | `{b.name}` | **{b.elo:.0f}** | {b.games} |\n"
      writer.add_text("arena/leaderboard", md, global_ep)
  except KeyboardInterrupt:
    print(f"\n[Train] Interrupted at episode {global_ep}. Last saved genertion is safe in '{arena.save_dir}/'.")
  finally:
    arena.close()
    writer.close()
