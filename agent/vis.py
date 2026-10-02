import torch
import numpy as np
import pygame

from state import State
from env import VecEnv
from dummy import DummyAgent

class RenderEnv(VecEnv):
  def __init__(self):
    super().__init__(1)

  def reset(self) -> State:
    s = super().reset()
    self._render()
    return s

  def step(self, a: torch.Tensor) -> State:
    r = super().step()
    self._render()
    return r

  def _render(self):
    if self.window is None:
      pygame.init()
      pygame.display.init()
      self.window = pygame.display.set_mode((1600, 900))
    if self.clock is None:
      self.clock = pygame.time.Clock()

    canvas = pygame.Surface((1600, 900))
    canvas.fill((255, 255, 255))
    
    state = self._state0_np
    scale = 1.0 / 10

    for i in range(4):
      x = float(state.x[..., i].item())
      y = float(state.y[..., i].item())
      c = (0, 255, 0) if i < 2 else (255, 0, 0)
      pygame.draw.circle(
        canvas,
        c,
        (x * scale, y * scale),
        400 * scale,  
      )
    
    self.window.blit(canvas, canvas.get_rect())
    pygame.event.pump()
    pygame.display.update()
    self.clock.tick(2)

def __name__ == "__main__":
  print("1")
  env = RenderEnv()
  print("2")
  agent0 = DummyAgent()
  agent1 = DummyAgent()

  print("3")
  s0, s1 = env.reset()
  print("4")
  for i in range(100):
    print("5")
    a0 = agent0.act(s0)
    a1 = agent1.act(s1)

    a = torch.cat([a0, a1], -1)

    print("6")
    s0, s1, reward, done = env.step(a)
    print("7")

