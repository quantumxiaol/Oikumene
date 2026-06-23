from fastapi import APIRouter

from oikumene_agent.agents.heuristic_agent import HeuristicAgent
from oikumene_agent.schemas.decision import DecisionBatchRequest, DecisionBatchResponse


router = APIRouter()
agent = HeuristicAgent()


@router.get("/health")
async def health() -> dict[str, str]:
    return {"status": "ok", "service": "OikumeneAgentServer"}


@router.post("/api/v1/decisions/batch", response_model=DecisionBatchResponse)
async def decide_batch(request: DecisionBatchRequest) -> DecisionBatchResponse:
    return agent.decide_batch(request)
