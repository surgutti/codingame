# Mad Pod Racing

## Build

```bash
make test
make env_server
make rl_test
```

## Validate

```bash
make validate
```

## Sandbox Arena

Run two bot binaries against the exact local referee and save a replay-style JSON file:

```bash
build/arena \
  --bot-a ./build/dummy_bot \
  --bot-b ./build/dummy_bot \
  --seed 745118001 \
  --output /tmp/replay.json.gz
```

You can also pass replay-style referee parameters directly:

```bash
build/arena \
  --bot-a ./build/dummy_bot \
  --bot-b ./build/dummy_bot \
  --referee-input-file replay.refereeinput \
  --output /tmp/replay.json.gz
```

`replay.refereeinput` is the same key=value format used in downloaded replay files:

```text
seed=745118001
pod_per_player=2
pod_timeout=100
map=3350 7254 14587 7720 10544 5070 13074 2316 4570 2157 7355 4935
```

## Targets

`make test`: builds the simulator, arena, and dummy bot, then runs an end-to-end smoke test.

`make validate`: runs the replay corpus against the exact simulator.

`make env_server`: builds the simulator-backed environment server used by the RL pipeline.

`make rl_test`: runs the RL/config/export/training smoke tests.

`make submission`: builds a submission only if [first_submission.cpp](/home/olaf/codingame/src/submission/first_submission.cpp) exists.

## RL Baseline

The phase-1 configurable DDQN baseline lives under [csb_rl](/home/olaf/codingame/csb_rl) with configs in [configs/default.yaml](/home/olaf/codingame/configs/default.yaml) and [configs/smoke.yaml](/home/olaf/codingame/configs/smoke.yaml).

Key commands:

```bash
.venv/bin/python scripts/train_ddqn.py --config configs/smoke.yaml
.venv/bin/python scripts/train_ddqn.py --config configs/fast_gpu.yaml
.venv/bin/python scripts/train_ddqn.py --config configs/resume_parallel.yaml
.venv/bin/python scripts/eval_ddqn.py --config configs/default.yaml --checkpoint runs/csb_runner_ddqn/best.pt
.venv/bin/python scripts/export_ddqn.py --config configs/default.yaml --checkpoint runs/csb_runner_ddqn/best.pt
.venv/bin/python scripts/benchmark_ddqn.py --mode replay-agreement
tensorboard --logdir runs
```

Detailed config and workflow documentation is in [rl_framework.md](/home/olaf/codingame/doc/rl_framework.md).
