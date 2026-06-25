#include "oikumene/sim/simulation_metrics.hpp"

#include <algorithm>

namespace oikumene {

float AverageAdminLoad(const Simulation& sim) {
    if (sim.Polities().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& polity : sim.Polities()) {
        total += polity.admin_load;
    }
    return total / static_cast<float>(sim.Polities().size());
}

float AverageAdminCapacity(const Simulation& sim) {
    if (sim.Polities().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& polity : sim.Polities()) {
        total += polity.admin_capacity;
    }
    return total / static_cast<float>(sim.Polities().size());
}

float AverageOverextension(const Simulation& sim) {
    if (sim.Polities().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& polity : sim.Polities()) {
        total += polity.overextension;
    }
    return total / static_cast<float>(sim.Polities().size());
}

float AverageStability(const Simulation& sim) {
    if (sim.Polities().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& polity : sim.Polities()) {
        total += polity.stability;
    }
    return total / static_cast<float>(sim.Polities().size());
}

float AverageUnlockedTechs(const Simulation& sim) {
    if (sim.Polities().empty()) {
        return 0.0F;
    }
    int unlocked = 0;
    for (const auto& polity : sim.Polities()) {
        unlocked += static_cast<int>(polity.research.unlocked.size());
    }
    return static_cast<float>(unlocked) / static_cast<float>(sim.Polities().size());
}

float AverageKnowledgeIncome(const Simulation& sim) {
    if (sim.Polities().empty()) {
        return 0.0F;
    }
    float total = 0.0F;
    for (const auto& polity : sim.Polities()) {
        total += polity.knowledge_income;
    }
    return total / static_cast<float>(sim.Polities().size());
}

float TechUnlockRate(const Simulation& sim, TechId tech) {
    if (sim.Polities().empty()) {
        return 0.0F;
    }
    int count = 0;
    for (const auto& polity : sim.Polities()) {
        count += HasTech(polity.research, tech) ? 1 : 0;
    }
    return static_cast<float>(count) / static_cast<float>(sim.Polities().size());
}

float AverageSettlementScore(const Simulation& sim) {
    if (sim.Settlements().empty()) {
        return 0.0F;
    }

    float total = 0.0F;
    for (const auto& settlement : sim.Settlements()) {
        total += sim.GetWorld().At(settlement.x, settlement.y).settlement_score;
    }
    return total / static_cast<float>(sim.Settlements().size());
}

float AverageSettlementFertility(const Simulation& sim) {
    if (sim.Settlements().empty()) {
        return 0.0F;
    }

    float total = 0.0F;
    for (const auto& settlement : sim.Settlements()) {
        total += sim.GetWorld().At(settlement.x, settlement.y).fertility;
    }
    return total / static_cast<float>(sim.Settlements().size());
}

int MaxSettlementPopulation(const Simulation& sim) {
    int maximum = 0;
    for (const auto& settlement : sim.Settlements()) {
        maximum = std::max(maximum, settlement.population);
    }
    return maximum;
}

int TotalPopulation(const Simulation& sim) {
    int total = 0;
    for (const auto& band : sim.Bands()) {
        if (band.active) {
            total += band.population;
        }
    }
    for (const auto& settlement : sim.Settlements()) {
        total += settlement.population;
    }
    return total;
}

} // namespace oikumene
