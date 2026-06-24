#pragma once

#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/event_log.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/trade.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

class TradeSystem {
  public:
    static void Reset(std::vector<TradeAgreement>& trades, std::vector<Polity>& polities);
    static void UpdateTrades(const World& world, Turn turn, const std::vector<Settlement>& settlements,
                             std::vector<Polity>& polities, std::vector<TradeAgreement>& trades, EventLog& event_log);
};

} // namespace oikumene
