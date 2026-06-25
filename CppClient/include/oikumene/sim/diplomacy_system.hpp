#pragma once

#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/diplomacy.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/trade.hpp"
#include "oikumene/sim/vassal_treaty.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

class DiplomacySystem {
  public:
    static void Reset(std::vector<DiplomacyRelation>& relations);
    static void UpdateDiplomacy(const World& world, Turn turn, const std::vector<Polity>& polities,
                                const std::vector<TradeAgreement>& trades,
                                const std::vector<VassalTreaty>& vassal_treaties,
                                std::vector<DiplomacyRelation>& relations);
    static void RecordIncident(std::vector<DiplomacyRelation>& relations, Turn turn, PolityId actor_id,
                               PolityId target_id, DiplomaticIncidentKind kind, float strength = 1.0F);
};

} // namespace oikumene
