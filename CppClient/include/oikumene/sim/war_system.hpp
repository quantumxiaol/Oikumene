#pragma once

#include <vector>

#include "oikumene/sim/event_log.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/trade.hpp"
#include "oikumene/sim/war_campaign.hpp"
#include "oikumene/sim/war_target.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

class WarSystem {
  public:
    static void Reset(std::vector<WarCampaign>& campaigns);

    static void UpdateWars(World& world, Turn turn, std::vector<Settlement>& settlements, std::vector<Polity>& polities,
                           const std::vector<TradeAgreement>& trades, const std::vector<WarTargetCandidate>& targets,
                           std::vector<WarCampaign>& campaigns, EventLog& event_log);
};

} // namespace oikumene
