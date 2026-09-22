#!/usr/bin/env python3

import torch
from env import VecEnv

def main():
  print("hello")

  envs = VecEnv(5, 42)
    
  print(envs.reset())

if __name__ == "__main__":
    main()
