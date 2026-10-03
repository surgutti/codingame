#!/usr/bin/env python3
import torch
import numpy as np
import pygame

from state import State
from env import VecEnv
from dummy import DummyAgent

class RenderEnv(VecEnv):
  def __init__(self, width=1280, height=720):
    super().__init__(1, seed=np.random.randint(0, 2**20))
    self.window = None
    self.clock = None
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

  def reset(self) -> State:
    s = super().reset()
    self._render()
    return s

  def step(self, a: torch.Tensor) -> State:
    r = super().step(a)
    self._render()
    return r

  def _world_to_screen(self, x: float, y: float):
    return int(x * self.scale), int(y * self.scale)

  def _render(self):
    if self.window is None:
      pygame.init()
      pygame.display.init()
      pygame.display.set_caption("Mad Pod Racing")
      self.window = pygame.display.set_mode((self.screen_w, self.screen_h), pygame.DOUBLEBUF)

    if self.clock is None:
      self.clock = pygame.time.Clock()
    
    for event in pygame.event.get():
      if event.type == pygame.QUIT or (event.type == pygame.KEYDOWN and event.key == pygame.K_ESCAPE):
          pygame.quit()
          exit(0)

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

    pod_radius_px = int(400 * self.scale)
    for i in range(4):
      px = float(state.x[..., i].item())
      py = float(state.y[..., i].item())
      vx = float(state.vx[..., i].item())
      vy = float(state.vy[..., i].item())
      c = self.COLOR_P1 if i < 2 else self.COLOR_P2
   
      sx, sy = self._world_to_screen(px, py) 
      pygame.draw.circle(
        canvas,
        c,
        (sx, sy),
        pod_radius_px 
      )

    self.window.blit(canvas, (0, 0))
    pygame.display.flip()
    self.clock.tick(15)

def visualize_fight(
  agent0: DummyAgent,
  agent1: DummyAgent
):
  env = RenderEnv()
  s0, s1 = env.reset()
  for i in range(300):
    a0 = agent0.act(s0)
    a1 = agent1.act(s1)

    a = torch.cat([a0, a1], -1)

    s0, s1, reward, done = env.step(a)
    if done.item():
      break

if __name__ == "__main__":
  agent0 = DummyAgent()
  agent1 = DummyAgent()

  visualize_fight(agent0, agent1)
