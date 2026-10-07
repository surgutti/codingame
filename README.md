# Mad Pod Racing

This repository is devoted to an old competition on CodinGame platform.

The goal of this project is to obtain top place in the leaderboard via
single Neural Network by enhanced Proximal Policy Optimization without any search methods.

My current progress is
[x] Pixel perfect simulation in C++
[x] Nanobind wrapper for Python
[x] Training harness
[x] SPO optimization (https://arxiv.org/abs/2401.16025)
[x] Reward potential (for future changes)
[ ] Winning with handmade agent (pending...)
[ ] First place

## Cloning

```bash
sudo apt install python3-dev
git clone --recurse-submodules -b mad-pod-racing https://github.com/surgutti/codingame.git
```

## Build

```bash
# change in the Makefile $PYTHON_VER if needed
make all
```

## Training

```bash
cd agent
python3 train.py
```
