#!/usr/bin/env python3
import torch
import numpy as np

from os import environ
environ['SDL_VIDEODRIVER'] = 'dummy'
environ['PYGAME_HIDE_SUPPORT_PROMPT'] = '1'
import pygame
import imageio

from state import State
from env import VecEnv
from agent import Agent

class RenderEnv(VecEnv):
  def __init__(self, device="cpu", width=1280, height=720):
    super().__init__(1, seed=np.random.randint(0, 2**20), device=device)
    self.screen_w = width
    self.screen_h = height

    self.world_w = 16000.0
    self.world_h = 9000.0
    self.scale = self.screen_w / self.world_w

    self.COLOR_BG = (15, 17, 23)
    self.COLOR_GRID = (30, 35, 45)
    self.COLOR_P1 = (0, 220, 255)
    self.COLOR_P2 = (255, 40, 80)
    self.COLOR_CP = (60, 65, 80)
    self.COLOR_TEXT = (220, 225, 235)

    pygame.init()
    pygame.font.init()
    pygame.display.set_mode((1, 1))

    self.font_large = pygame.font.SysFont("monospace", 24, bold=True)
    self.font_small = pygame.font.SysFont("monospace", 16)

  def reset(self) -> State:
    self.step_count = 0
    return super().reset()

  def step(self, a: torch.Tensor) -> State:
    self.step_count += 1
    return super().step(a)

  def _world_to_screen(self, x: float, y: float):
    return int(x * self.scale), int(y * self.scale)

  def render(self, agent_names = None, reward: float = 0) -> np.ndarray:
    canvas = pygame.Surface((self.screen_w, self.screen_h))
    canvas.fill(self.COLOR_BG)
    
    state = State(self.state0_cpu)

    for gx in range(0, int(self.world_w), 1000):
      sx, _ = self._world_to_screen(gx, 0)
      pygame.draw.line(
        canvas,
        self.COLOR_GRID,
        (sx, 0),
        (sx, self.screen_h),
        1
      )

    for gy in range(0, int(self.world_h), 1000):
      _, sy = self._world_to_screen(0, gy)
      pygame.draw.line(
        canvas,
        self.COLOR_GRID,
        (0, sy),
        (self.screen_w, sy),
        1
      )

    num_cps = state.num_cps
    cps_x = state.checkpoints[..., :num_cps, 0].squeeze().tolist()
    cps_y = state.checkpoints[..., :num_cps, 1].squeeze().tolist()
      
    cp_radius_px = int(600 * self.scale)
    for i in range(state.num_cps.item()):
      cx, cy = cps_x[i], cps_y[i]
      x, y = self._world_to_screen(cx, cy)

      pygame.draw.circle(
        canvas,
        self.COLOR_CP,
        (x, y),
        cp_radius_px,
        2
      )

      cp_text = self.font_small.render(str(i), True, (100, 110, 130))
      canvas.blit(cp_text, cp_text.get_rect(center=(x, y)))

    pod_radius_px = int(400 * self.scale)
    for i in range(4):
      px = float(state.x[..., i].item())
      py = float(state.y[..., i].item())
      vx = float(state.vx[..., i].item())
      vy = float(state.vy[..., i].item())
      fx = float(np.cos(state.angle[..., i].item())) * 800
      fy = float(np.sin(state.angle[..., i].item())) * 800
      c = self.COLOR_P1 if i < 2 else self.COLOR_P2
   
      sx, sy = self._world_to_screen(px, py) 
      pygame.draw.circle(
        canvas,
        c,
        (sx, sy),
        pod_radius_px 
      )
      
      sfx, sfy = self._world_to_screen(px + fx, py + fy)

      pygame.draw.line(
        canvas,
        c,
        (sx, sy),
        (sfx, sfy),
        3
      )

      svx, svy = self._world_to_screen(px + vx, py + vy)

      pygame.draw.line(
        canvas,
        (255, 0, 0),
        (sx, sy),
        (svx, svy),
        3
      )

      next_cp = int(state.next_cp[..., i].item())
      pod_label = self.font_small.render(f"P{i} (CP:{next_cp})", True, self.COLOR_TEXT)
      canvas.blit(pod_label, (sx + pod_radius_px + 8, sy - 10))

    hud_surface = pygame.Surface((self.screen_w, 50), pygame.SRCALPHA)
    hud_surface.fill((10, 12, 16, 200))

    step_text = self.font_large.render(f"Step: {self.step_count}", True, self.COLOR_TEXT)
    hud_surface.blit(step_text, (20, 12))

    reward_text = self.font_large.render(f"Reward: {reward:.4f}", True, self.COLOR_TEXT)
    hud_surface.blit(reward_text, (self.screen_w - 250, 12))

    if agent_names is not None:
      match_title = self.font_large.render(f"{agent_names[0]} vs {agent_names[1]}", True, self.COLOR_TEXT)
      hud_surface.blit(match_title, match_title.get_rect(center=(self.screen_w // 2, 25)))

    canvas.blit(hud_surface, (0, 0))

    frame = pygame.surfarray.array3d(canvas)
    return np.transpose(frame, (1, 0, 2))

def visualize_fight(
  agent0: Agent,
  agent1: Agent,
  save_path: str,
  agent_names = None,
  device = "cpu"
) -> np.ndarray:
  env = RenderEnv(device=device)
  s0, s1 = env.reset()

  frames = []
  frames.append(env.render(agent_names))

  for i in range(500):
    a0 = agent0.act(s0)
    a1 = agent1.act(s1)

    a = torch.cat([a0, a1], -1)

    s0, s1, reward, done = env.step(a)
    frames.append(env.render(agent_names, float(reward.item())))

    if done.item():
      break
 
  imageio.mimwrite(save_path, frames, format='FFMPEG', fps=15, macro_block_size=1) 

  vid = np.array(frames)
  vid = np.transpose(vid, (0, 3, 1, 2))
  tb_video = np.expand_dims(vid, axis=0)

  return tb_video

if __name__ == "__main__":
  from handmade.handmade import HandmadeAgent
  from handmade.handmade2 import Handmade2Agent
  from dummy.dummy import DummyAgent

  agent0 = DummyAgent()
  agent1 = DummyAgent() # Handmade2Agent()

  tb_vid = visualize_fight(
    agent0, 
    agent1, 
    save_path="replays/match.mp4",
    agent_names=["Dummy1", "Dummy2"]
  )

  from torch.utils.tensorboard import SummaryWriter
  writer = SummaryWriter("runs/pod_racing")
  writer.add_video("eval/match", tb_vid, global_step=0, fps=15)

  writer.close()
