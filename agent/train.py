#!/usr/bin/env python3

import os
import torch
import time

from torch.utils.tensorboard import SummaryWriter
# from torch.profiler import profile, record_function, ProfilerActivity
from tqdm import tqdm

from arena import Arena, BotEntry
from env import VecEnv, RAW_STATE_DIM
from agent import Agent
from spo.config import SPOConfig
from spo.spo import SPOAgent
from handmade.handmade import HandmadeAgent
from handmade.handmade2 import Handmade2Agent
from dummy.dummy import DummyAgent
from state import State
from stats import CUSTOM_LAYOUT
from vis import visualize_fight
from config import Config

def train(
  config: Config,
  writer: SummaryWriter,
  envs: VecEnv,
  agent0: Agent,
  arena: Arena,
  num_episodes: int,
  start_episode: int
):
  opponent_weights = arena.nash_weights()

  with tqdm(
    range(start_episode, start_episode + num_episodes),
    initial=start_episode,
    total=start_episode + num_episodes,
    desc="Training",
    unit="ep"
  ) as pbar:
    for episode in pbar:
      state0, state1 = envs.reset()

      opp_entry, agent1 = arena.sample_opponent(weights=opponent_weights)

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
        start = time.perf_counter()

        with torch.no_grad():
          action0 = agent0.act(state0, training=True)
          action1 = agent1.act(state1) #training=True)
    
        env_action = torch.cat([action0, action1], -1)

        inference_time += (time.perf_counter() - start)

        start = time.perf_counter()
        next_state0, next_state1, reward, done = envs.step(env_action)

        agent0.observe(+reward, next_state0, done)
        # agent1.observe(-reward, next_state1, done)

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
  config = Config()
  envs = VecEnv(config.num_envs, config.seed, config.device)
  arena = Arena(config, envs)

  agent_config = SPOConfig 
  agent0 = SPOAgent(agent_config).to(config.device)

  global_ep = 0
  start_gen, global_ep = arena.load_state(agent0)

  if len(arena.pool) == 0:
    arena.register("Handmade1", HandmadeAgent())
    arena.register("Handmade2", Handmade2Agent())
    arena.register("Dummy", DummyAgent())

  writer = SummaryWriter(
    log_dir=f"runs/dashboard",
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
        num_episodes=config.episodes_per_gen, 
        start_episode=global_ep
      )
      global_ep += config.episodes_per_gen

      is_champion, wr = arena.is_champion(agent0)
      print(f"Learner have win ratio: {wr=} against the Nash")
      if is_champion:
        print("Adding learner to the leaderboard")
        frozen_learner = SPOAgent(agent_config).to(config.device)
        frozen_learner.load_state_dict(agent0.state_dict())
        frozen_learner.eval()

        ckpt_name = f"{agent_config.name}_{gen:04d}"
        ckpt_entry = arena.register(
          name=ckpt_name,
          agent=frozen_learner,
          agent_kwargs={"config": agent_config}
        )

        tb_vid = visualize_fight(
          agent0,
          Handmade2Agent(),
          save_path=f"replays/match_{ckpt_name}.mp4",
          agent_names=["Learner", "Handmade2"],
          device=config.device
        )
        writer.add_video("replays/match", tb_vid, global_step=global_ep, fps=15)

        arena.save_state(
          agent0, 
          next_gen=gen + 1, 
          global_ep=global_ep
        )

  except KeyboardInterrupt:
    print(f"\n[Train] Interrupted at episode {global_ep}. Last saved genertion is safe in '{arena.save_dir}/'.")
  finally:
    arena.close()
    writer.close()
