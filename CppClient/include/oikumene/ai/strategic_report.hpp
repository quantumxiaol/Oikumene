#pragma once

#include <vector>

#include <nlohmann/json.hpp>

#include "oikumene/ai/decision_models.hpp"
#include "oikumene/core/simulation.hpp"

namespace oikumene {

struct StrategicReportOptions {
    int max_neighbors = 4;
    int max_war_pressures = 3;
    int max_war_targets = 3;
    int max_wars = 3;
    int max_occupations = 3;
    int max_vassal_treaties = 3;
    int max_recent_events = 6;
    int max_candidate_actions = 8;
};

struct StrategicReport {
    std::string version = "0.1";
    Turn turn = 0;
    PolityId polity_id = kInvalidPolityId;
    nlohmann::json report = nlohmann::json::object();
    std::vector<CandidateAction> candidate_actions;
};

[[nodiscard]] StrategicReport BuildStrategicReport(const Simulation& sim, PolityId polity_id,
                                                   const StrategicReportOptions& options = {});
[[nodiscard]] std::vector<StrategicReport> BuildStrategicReports(const Simulation& sim,
                                                                 const StrategicReportOptions& options = {});
[[nodiscard]] DecisionRequest BuildDecisionRequest(const StrategicReport& report, Turn strategic_turn);
[[nodiscard]] DecisionBatchRequest BuildDecisionBatchRequest(const Simulation& sim, Turn strategic_turn,
                                                             const StrategicReportOptions& options = {});

[[nodiscard]] nlohmann::json ToJson(const StrategicReport& report);

} // namespace oikumene
