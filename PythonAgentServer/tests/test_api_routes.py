import asyncio

from oikumene_agent.api.routes import decide_batch, health, router
from oikumene_agent.schemas.decision import CandidateAction, DecisionBatchRequest, DecisionRequest


def run_async(coro):
    return asyncio.run(coro)


def test_health_route_is_registered() -> None:
    routes = {(route.path, tuple(sorted(route.methods or []))) for route in router.routes}

    assert ("/health", ("GET",)) in routes
    assert ("/api/v1/decisions/batch", ("POST",)) in routes


def test_health_response() -> None:
    assert run_async(health()) == {"status": "ok", "service": "OikumeneAgentServer"}


def test_decide_batch_selects_highest_net_benefit_action() -> None:
    request = DecisionBatchRequest(
        requests=[
            DecisionRequest(
                request_id="turn_12_polity_3",
                turn=12,
                strategic_turn=2,
                polity_id=3,
                candidate_actions=[
                    CandidateAction(
                        action_id="research_pottery",
                        type="RESEARCH_TECH",
                        estimated_benefit=0.55,
                        risk=0.05,
                    ),
                    CandidateAction(
                        action_id="open_trade_7",
                        type="OPEN_TRADE",
                        estimated_benefit=0.75,
                        risk=0.10,
                    ),
                    CandidateAction(
                        action_id="prepare_war_target_4",
                        type="PREPARE_WAR",
                        estimated_benefit=0.95,
                        risk=0.65,
                    ),
                ],
            )
        ]
    )

    response = run_async(decide_batch(request))

    assert response.protocol_version == "0.1"
    assert len(response.responses) == 1

    decision = response.responses[0]
    assert decision.request_id == "turn_12_polity_3"
    assert decision.turn == 12
    assert decision.polity_id == 3
    assert len(decision.selected_actions) == 1
    assert decision.selected_actions[0].action_id == "open_trade_7"
    assert abs(decision.selected_actions[0].priority - 0.65) < 1e-6
