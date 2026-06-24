#pragma once

#include <vector>

#include "oikumene/sim/diplomacy.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/war.hpp"

namespace oikumene {

[[nodiscard]] std::vector<WarPressure> BuildWarPressures(const std::vector<Polity>& polities,
                                                         const std::vector<DiplomacyRelation>& relations);

} // namespace oikumene
