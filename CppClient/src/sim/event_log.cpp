#include "oikumene/sim/event_log.hpp"

namespace oikumene {

void EventLog::Add(SimEvent event) {
    events_.push_back(std::move(event));
}

const std::vector<SimEvent>& EventLog::Events() const {
    return events_;
}

std::vector<SimEvent>& EventLog::Events() {
    return events_;
}

std::size_t EventLog::Size() const {
    return events_.size();
}

} // namespace oikumene
