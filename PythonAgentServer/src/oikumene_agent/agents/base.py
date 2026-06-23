from abc import ABC, abstractmethod

from oikumene_agent.schemas.decision import DecisionBatchRequest, DecisionBatchResponse


class Agent(ABC):
    @abstractmethod
    def decide_batch(self, request: DecisionBatchRequest) -> DecisionBatchResponse:
        """Return suggested actions for each strategic request."""
