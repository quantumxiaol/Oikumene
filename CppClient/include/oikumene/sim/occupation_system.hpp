#pragma once

#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/event_log.hpp"
#include "oikumene/sim/occupation.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/war_campaign.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

class OccupationSystem {
  public:
    static void Reset(std::vector<OccupationRecord>& occupations, std::vector<Polity>& polities);

    static void UpdateOccupations(World& world, Turn turn, std::vector<Settlement>& settlements,
                                  std::vector<Polity>& polities, const std::vector<WarCampaign>& campaigns,
                                  std::vector<OccupationRecord>& occupations, EventLog& event_log);
};

} // namespace oikumene
