#pragma once

#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/diplomacy.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/trade.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

class DiplomacySystem {
  public:
    static void Reset(std::vector<DiplomacyRelation>& relations);
    static void UpdateDiplomacy(const World& world, Turn turn, const std::vector<Polity>& polities,
                                const std::vector<TradeAgreement>& trades, std::vector<DiplomacyRelation>& relations);
};

} // namespace oikumene
