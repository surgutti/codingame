
CUSTOM_LAYOUT = {
  "Game": {
      "Rates": ["Multiline", ["game/win_rate", "game/loss_rate", "game/draw_rate"]],
  },
  "Actions": {
    "Steer": ["Multiline", ["actions/steer_left_frac", "actions/steer_straight_frac", "actions/steer_right_frac"]],
    "Thrust": ["Multiline", ["actions/thrust_0_frac", "actions/thrust_200_frac", "actions/shield_frac"]]
  },
  "Perf": {
    "Timing": ["Multiline", ["perf/inference", "perf/simulation", "perf/update"]]
  }
}
