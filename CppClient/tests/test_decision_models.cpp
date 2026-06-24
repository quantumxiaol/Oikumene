#include <cassert>
#include <iostream>

#include "oikumene/ai/heuristic_decision_provider.hpp"
#include "oikumene/io/json_protocol.hpp"

namespace {

void TestDecisionModels() {
    using namespace oikumene;

    CandidateAction action{
        .action_id = "research_mining",
        .type = ActionType::ResearchTech,
        .target = "mining",
        .cost = 40.0,
        .estimated_benefit = 0.8,
        .risk = 0.1,
    };

    DecisionRequest request;
    request.request_id = "turn_10_polity_1";
    request.turn = 10;
    request.strategic_turn = 2;
    request.polity_id = 1;
    request.report = Json{{"name", "Test Polity"}};
    request.candidate_actions.push_back(action);

    const auto serialized = SerializeDecisionRequest(request);
    const auto parsed = serialized.get<DecisionRequest>();
    assert(parsed.request_id == request.request_id);
    assert(parsed.candidate_actions.size() == 1);
    assert(parsed.candidate_actions.front().type == ActionType::ResearchTech);

    DecisionBatchRequest batch_request;
    batch_request.requests.push_back(request);

    HeuristicDecisionProvider provider;
    const auto response = provider.DecideBatch(batch_request);
    assert(response.responses.size() == 1);
    assert(response.responses.front().selected_actions.size() == 1);
    assert(response.responses.front().selected_actions.front().action_id == "research_mining");
}

} // namespace

int main() {
    TestDecisionModels();

    std::cout << "oikumene_decision_tests passed\n";
    return 0;
}
