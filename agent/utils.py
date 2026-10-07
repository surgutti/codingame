#!/usr/bin/env python3

import numpy as np
from scipy.optimize import linprog

def calculate_nash_weights(
  wins: np.ndarray,
  draws: np.ndarray,
  losses: np.ndarray
) -> np.ndarray:
  
  games = np.clip(wins + draws + losses, a_min=1, a_max=None)

  M = (wins - losses) / games
  n = M.shape[0]

  c = np.zeros(n + 1)
  c[-1] = -1.0

  A_ub = np.hstack([-M.T, np.ones((n, 1))])
  b_ub = np.zeros(n)

  A_eq = np.ones((1, n + 1))
  A_eq[0, -1] = 0.0
  b_eq = np.array([1.0])

  bounds = [(0.0, 1.0) for _ in range(n)] + [(-1.0, +1.0)]

  res = linprog(
    c,
    A_ub=A_ub,
    b_ub=b_ub,
    A_eq=A_eq,
    b_eq=b_eq,
    bounds=bounds,
    method='highs'
  )
  
  assert res.success

  mu = res.x[:-1]
  mu = np.clip(mu, 0.0, 1.0)

  return mu / np.sum(mu)

if __name__ == "__main__":

  wins = np.array(
      [[0, 1, 0],
       [0, 0, 1],
       [1, 0, 0]]
  )
  draws = np.array(
      [[0, 0, 0],
       [0, 0, 0],
       [0, 0, 0]]
  )
  losses = np.array(
      [[0, 0, 1],
       [1, 0, 0],
       [0, 1, 0]]
  )

  weights = calculate_nash_weights(wins, draws, losses)
  print(weights)

  print("Ok")
