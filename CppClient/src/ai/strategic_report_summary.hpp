#pragma once

#include <nlohmann/json.hpp>

#include "oikumene/ai/strategic_report.hpp"

namespace oikumene::strategic_report {

[[nodiscard]] nlohmann::json StrategicSummaryToJson(const Simulation& sim, const Polity& polity,
                                                    const StrategicReportOptions& options);

} // namespace oikumene::strategic_report
