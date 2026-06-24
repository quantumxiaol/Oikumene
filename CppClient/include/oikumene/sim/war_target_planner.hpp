#pragma once

#include <vector>

#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/trade.hpp"
#include "oikumene/sim/war.hpp"
#include "oikumene/sim/war_target.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

[[nodiscard]] std::vector<WarTargetCandidate> BuildWarTargetCandidates(const World& world,
                                                                       const std::vector<Settlement>& settlements,
                                                                       const std::vector<Polity>& polities,
                                                                       const std::vector<TradeAgreement>& trades,
                                                                       const std::vector<WarPressure>& pressures);

} // namespace oikumene
