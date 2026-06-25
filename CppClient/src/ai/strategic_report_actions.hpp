#pragma once

#include <vector>

#include "oikumene/ai/strategic_report.hpp"

namespace oikumene::strategic_report {

[[nodiscard]] std::vector<CandidateAction> BuildCandidateActions(const Simulation& sim, const Polity& polity,
                                                                 const StrategicReportOptions& options);

} // namespace oikumene::strategic_report
