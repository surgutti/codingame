from spo.spo import SPOAgent
from dummy.dummy import DummyAgent
from handmade.handmade import HandmadeAgent
from handmade.handmade2 import Handmade2Agent

type Agent = SPOAgent | DummyAgent | HandmadeAgent | Handmad2Agent

AGENT_REGISTRY = {
  "SPOAgent": SPOAgent,
  "HandmadeAgent": HandmadeAgent,
  "Handmade2Agent": Handmade2Agent,
  "DummyAgent": DummyAgent,
}
