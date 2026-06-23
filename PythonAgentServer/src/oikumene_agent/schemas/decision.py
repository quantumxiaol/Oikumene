from __future__ import annotations

from typing import Any, Literal

from pydantic import BaseModel, ConfigDict, Field


PROTOCOL_VERSION = "0.1"

ActionType = Literal[
    "RESEARCH_TECH",
    "OPEN_TRADE",
    "PREPARE_WAR",
    "DECLARE_WAR",
    "OFFER_PEACE",
    "UNKNOWN",
]


class CandidateAction(BaseModel):
    model_config = ConfigDict(extra="allow")

    action_id: str
    type: ActionType = "UNKNOWN"
    target: str = ""
    cost: float = 0.0
    estimated_benefit: float = 0.0
    risk: float = 0.0


class DecisionRequest(BaseModel):
    model_config = ConfigDict(extra="allow")

    protocol_version: str = PROTOCOL_VERSION
    request_id: str
    turn: int = 0
    strategic_turn: int = 0
    polity_id: int = -1
    report: dict[str, Any] = Field(default_factory=dict)
    candidate_actions: list[CandidateAction] = Field(default_factory=list)


class SelectedAction(BaseModel):
    action_id: str
    priority: float = 0.0
    commitment: Literal["low", "medium", "high"] = "medium"


class DiplomaticMessage(BaseModel):
    target_polity_id: int = -1
    intent: str = ""
    message: str = ""


class DecisionResponse(BaseModel):
    protocol_version: str = PROTOCOL_VERSION
    request_id: str
    turn: int = 0
    polity_id: int = -1
    selected_actions: list[SelectedAction] = Field(default_factory=list)
    diplomatic_messages: list[DiplomaticMessage] = Field(default_factory=list)
    rationale_short: str = ""
    confidence: float = 0.0


class DecisionBatchRequest(BaseModel):
    protocol_version: str = PROTOCOL_VERSION
    requests: list[DecisionRequest] = Field(default_factory=list)


class DecisionBatchResponse(BaseModel):
    protocol_version: str = PROTOCOL_VERSION
    responses: list[DecisionResponse] = Field(default_factory=list)
