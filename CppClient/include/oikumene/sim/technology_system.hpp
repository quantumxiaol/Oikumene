#pragma once

#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/event_log.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

[[nodiscard]] float KnowledgeIncomeFor(const World& world, const std::vector<Settlement>& settlements,
                                       const Polity& polity);
[[nodiscard]] bool CanResearchTech(const World& world, const std::vector<Settlement>& settlements, const Polity& polity,
                                   TechId tech);
[[nodiscard]] TechId ChooseResearchFor(const World& world, const std::vector<Settlement>& settlements,
                                       const Polity& polity);

class TechnologySystem {
  public:
    static void UpdateTechnologies(const World& world, Turn turn, const std::vector<Settlement>& settlements,
                                   std::vector<Polity>& polities, EventLog& event_log);
};

} // namespace oikumene
