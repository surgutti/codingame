import os
import json
import shutil

from torch.utils.tensorboard import SummaryWriter
from dataclasses import dataclass, field, asdict, is_dataclass

from agent import Agent

# fix for saving the entry and configs
class JSONFix(json.JSONEncoder):
  def default(self, obj):
    if is_dataclass(obj):
      return asdict(obj)
    if hasattr(obj, "__dict__"):
      return obj.__dict__
    return super().default(obj)

@dataclass
class BotEntry:
  name: str
  path: str | None
  log_dir: str
  pool_idx: int
  agent_type: str
  agent_kwargs: dict = field(default_factory=dict)
  purge_step: int | None = field(default=None, repr=False)
  writer: SummaryWriter | None = field(init=False, default=None, repr=False)

  _instance: Agent | None = field(init=False, default=None, repr=False)

  def log_step(self, global_ep: int, rank: int):
    if self.log_dir is None:
      return
    if self.writer is None:
      self.writer = SummaryWriter(log_dir=self.log_dir, purge_step=self.purge_step)

  def to_dict(self) -> dict:
    return {
      "name": self.name,
      "path": self.path,
      "log_dir": self.log_dir,
      "pool_idx": self.pool_idx,
      "agent_type": self.agent_type,
      "agent_kwargs": self.agent_kwargs,
    }

  def cleanup(self):
    if self.writer is not None:
      self.writer.close()
      self.writer = None
    if self.path and os.path.exists(self.path):
      os.remove(self.path)
    if self.log_dir and os.path.exists(self.log_dir):
      shutil.rmtree(self.log_dir, ignore_errors=True)
    self._instance = None
