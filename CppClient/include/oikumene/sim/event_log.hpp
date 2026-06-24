#pragma once

#include <vector>

#include "oikumene/sim/event.hpp"

namespace oikumene {

class EventLog {
  public:
    void Add(SimEvent event);
    [[nodiscard]] const std::vector<SimEvent>& Events() const;
    [[nodiscard]] std::vector<SimEvent>& Events();
    [[nodiscard]] std::size_t Size() const;

  private:
    std::vector<SimEvent> events_;
};

} // namespace oikumene
