from oikumene_agent.agents.base import Agent
from oikumene_agent.schemas.decision import (
    DecisionBatchRequest,
    DecisionBatchResponse,
    DecisionResponse,
    SelectedAction,
)


class HeuristicAgent(Agent):
    def decide_batch(self, request: DecisionBatchRequest) -> DecisionBatchResponse:
        responses: list[DecisionResponse] = []

        for item in request.requests:
            selected_actions: list[SelectedAction] = []
            if item.candidate_actions:
                best_action = max(
                    item.candidate_actions,
                    key=lambda action: action.estimated_benefit - action.risk,
                )
                selected_actions.append(
                    SelectedAction(
                        action_id=best_action.action_id,
                        priority=max(0.0, min(1.0, best_action.estimated_benefit - best_action.risk)),
                        commitment="medium",
                    )
                )

            responses.append(
                DecisionResponse(
                    protocol_version=request.protocol_version,
                    request_id=item.request_id,
                    turn=item.turn,
                    polity_id=item.polity_id,
                    selected_actions=selected_actions,
                    rationale_short="Mock heuristic response: selected the highest benefit minus risk candidate action.",
                    confidence=0.55,
                )
            )

        return DecisionBatchResponse(protocol_version=request.protocol_version, responses=responses)
