#include "oikumene/ai/heuristic_decision_provider.hpp"

#include <algorithm>

namespace oikumene {

std::string HeuristicDecisionProvider::Name() const {
    return "heuristic";
}

HealthStatus HeuristicDecisionProvider::CheckHealth() {
    return HealthStatus{
        .online = true,
        .status_code = 200,
        .service = "LocalHeuristicDecisionProvider",
        .message = "ok",
        .latency_ms = 0,
    };
}

DecisionBatchResponse HeuristicDecisionProvider::DecideBatch(const DecisionBatchRequest& request) {
    DecisionBatchResponse batch_response;
    batch_response.protocol_version = request.protocol_version;

    for (const auto& decision_request : request.requests) {
        DecisionResponse response;
        response.protocol_version = request.protocol_version;
        response.request_id = decision_request.request_id;
        response.turn = decision_request.turn;
        response.polity_id = decision_request.polity_id;
        response.rationale_short = "Selected the highest benefit candidate action from the local heuristic provider.";
        response.confidence = 0.55;

        if (!decision_request.candidate_actions.empty()) {
            const auto best = std::max_element(
                decision_request.candidate_actions.begin(),
                decision_request.candidate_actions.end(),
                [](const CandidateAction& lhs, const CandidateAction& rhs) {
                    return (lhs.estimated_benefit - lhs.risk) < (rhs.estimated_benefit - rhs.risk);
                });

            response.selected_actions.push_back(SelectedAction{
                .action_id = best->action_id,
                .priority = std::clamp(best->estimated_benefit - best->risk, 0.0, 1.0),
                .commitment = "medium",
            });
        }

        batch_response.responses.push_back(response);
    }

    return batch_response;
}

}  // namespace oikumene
