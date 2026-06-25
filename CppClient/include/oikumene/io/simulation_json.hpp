#pragma once

#include <cstddef>

#include <nlohmann/json.hpp>

#include "oikumene/sim/event.hpp"
#include "oikumene/sim/event_log.hpp"

namespace oikumene {

[[nodiscard]] nlohmann::json ToJson(const SimEvent& event);
[[nodiscard]] nlohmann::json RecentEventsToJson(const EventLog& events, std::size_t max_count);

} // namespace oikumene
