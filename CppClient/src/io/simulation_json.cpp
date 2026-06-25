#include "oikumene/io/simulation_json.hpp"

#include "oikumene/sim/event.hpp"

namespace oikumene {

nlohmann::json ToJson(const SimEvent& event) {
    return nlohmann::json{
        {"turn", event.turn}, {"type", ToString(event.type)}, {"actor_id", event.actor_id}, {"x", event.x},
        {"y", event.y},       {"summary", event.summary},
    };
}

nlohmann::json RecentEventsToJson(const EventLog& events, std::size_t max_count) {
    nlohmann::json values = nlohmann::json::array();
    const auto& all_events = events.Events();
    const std::size_t first = all_events.size() > max_count ? all_events.size() - max_count : 0;
    for (std::size_t i = first; i < all_events.size(); ++i) {
        values.push_back(ToJson(all_events[i]));
    }
    return values;
}

} // namespace oikumene
