#include "oikumene/ai/strategic_report.hpp"

#include <sstream>

#include "strategic_report_actions.hpp"
#include "strategic_report_helpers.hpp"
#include "strategic_report_summary.hpp"

namespace oikumene {

StrategicReport BuildStrategicReport(const Simulation& sim, PolityId polity_id, const StrategicReportOptions& options) {
    const auto* polity = strategic_report::PolityById(sim.Polities(), polity_id);
    StrategicReport result;
    result.turn = sim.CurrentTurn();
    result.polity_id = polity_id;
    if (polity == nullptr) {
        result.report = nlohmann::json{{"version", result.version},
                                       {"turn", result.turn},
                                       {"polity_id", polity_id},
                                       {"error", "polity not found"}};
        return result;
    }

    result.report = strategic_report::StrategicSummaryToJson(sim, *polity, options);
    result.candidate_actions = strategic_report::BuildCandidateActions(sim, *polity, options);
    return result;
}

std::vector<StrategicReport> BuildStrategicReports(const Simulation& sim, const StrategicReportOptions& options) {
    std::vector<StrategicReport> reports;
    reports.reserve(sim.Polities().size());
    for (const auto& polity : sim.Polities()) {
        reports.push_back(BuildStrategicReport(sim, polity.id, options));
    }
    return reports;
}

DecisionRequest BuildDecisionRequest(const StrategicReport& report, Turn strategic_turn) {
    std::ostringstream request_id;
    request_id << "turn_" << report.turn << "_polity_" << report.polity_id;
    DecisionRequest request;
    request.request_id = request_id.str();
    request.turn = report.turn;
    request.strategic_turn = strategic_turn;
    request.polity_id = report.polity_id;
    request.report = report.report;
    request.candidate_actions = report.candidate_actions;
    return request;
}

DecisionBatchRequest BuildDecisionBatchRequest(const Simulation& sim, Turn strategic_turn,
                                               const StrategicReportOptions& options) {
    DecisionBatchRequest batch;
    for (const auto& report : BuildStrategicReports(sim, options)) {
        batch.requests.push_back(BuildDecisionRequest(report, strategic_turn));
    }
    return batch;
}

nlohmann::json ToJson(const StrategicReport& report) {
    return nlohmann::json{
        {"version", report.version},
        {"turn", report.turn},
        {"polity_id", report.polity_id},
        {"report", report.report},
        {"candidate_actions", report.candidate_actions},
    };
}

} // namespace oikumene
