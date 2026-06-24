#include "oikumene/sim/technology_system.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "oikumene/sim/tech_effects.hpp"

namespace oikumene {
namespace {

const Settlement* SettlementById(const std::vector<Settlement>& settlements, int id) {
    for (const auto& settlement : settlements) {
        if (settlement.id == id) {
            return &settlement;
        }
    }
    return nullptr;
}

bool IsMineral(ResourceKind resource) {
    return resource == ResourceKind::Copper || resource == ResourceKind::Tin || resource == ResourceKind::ShallowIron ||
           resource == ResourceKind::ShallowCoal || resource == ResourceKind::Gold ||
           resource == ResourceKind::Silver || resource == ResourceKind::MeteoricIron;
}

std::vector<const Settlement*> MemberSettlements(const std::vector<Settlement>& settlements, const Polity& polity) {
    std::vector<const Settlement*> members;
    for (const int id : polity.member_settlement_ids) {
        if (const auto* settlement = SettlementById(settlements, id)) {
            members.push_back(settlement);
        }
    }
    return members;
}

int TileIndex(const World& world, int x, int y) {
    return y * world.Width() + x;
}

std::vector<int> NearbyTileIndices(const World& world, const Settlement& settlement, int radius = 3) {
    std::vector<int> indices;
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            const int x = settlement.x + dx;
            const int y = settlement.y + dy;
            if (world.InBounds(x, y)) {
                indices.push_back(TileIndex(world, x, y));
            }
        }
    }
    return indices;
}

bool HasCoastalAccess(const World& world, const std::vector<Settlement>& settlements, const Polity& polity) {
    for (const auto* settlement : MemberSettlements(settlements, polity)) {
        if (world.At(settlement->x, settlement->y).is_coast) {
            return true;
        }
        for (const int index : NearbyTileIndices(world, *settlement)) {
            if (world.Tiles()[static_cast<std::size_t>(index)].is_coast) {
                return true;
            }
        }
    }
    for (const auto& tile : world.Tiles()) {
        if (tile.controller_polity_id == polity.id && tile.is_coast) {
            return true;
        }
    }
    return false;
}

int MinePotential(const World& world, const std::vector<Settlement>& settlements, const Polity& polity) {
    int potential = 0;
    for (const auto* settlement : MemberSettlements(settlements, polity)) {
        for (const int index : NearbyTileIndices(world, *settlement)) {
            const auto& tile = world.Tiles()[static_cast<std::size_t>(index)];
            potential += IsMineral(tile.resource) ? 1 : 0;
            potential += tile.improvement == ImprovementKind::ShallowMine ? 2 : 0;
        }
    }
    return potential;
}

int PastureOrHorsePotential(const World& world, const std::vector<Settlement>& settlements, const Polity& polity) {
    int potential = 0;
    for (const auto* settlement : MemberSettlements(settlements, polity)) {
        for (const int index : NearbyTileIndices(world, *settlement)) {
            const auto& tile = world.Tiles()[static_cast<std::size_t>(index)];
            potential += tile.resource == ResourceKind::Horse ? 2 : 0;
            potential += tile.improvement == ImprovementKind::Pasture ? 1 : 0;
        }
    }
    return potential;
}

int RiverFarmPotential(const World& world, const std::vector<Settlement>& settlements, const Polity& polity) {
    int potential = 0;
    for (const auto* settlement : MemberSettlements(settlements, polity)) {
        for (const int index : NearbyTileIndices(world, *settlement)) {
            const auto& tile = world.Tiles()[static_cast<std::size_t>(index)];
            if (tile.improvement == ImprovementKind::Farm && tile.has_river) {
                potential += 2;
            } else if (tile.soil_quality >= 0.55F && tile.has_river) {
                ++potential;
            }
        }
    }
    return potential;
}

bool FoodPressureHigh(const std::vector<Settlement>& settlements, const Polity& polity) {
    if (polity.budget.food_surplus < 0.0F) {
        return true;
    }
    for (const auto* settlement : MemberSettlements(settlements, polity)) {
        const float reserve = settlement->stockpile.food / std::max(1.0F, static_cast<float>(settlement->population));
        if (reserve < 0.45F || settlement->carrying_capacity_ratio > 0.78F) {
            return true;
        }
    }
    return false;
}

bool HasBronzeMaterialAccess(const World& world, const std::vector<Settlement>& settlements, const Polity& polity) {
    if (polity.budget.ore_income >= 0.25F || polity.ore >= 5.0F) {
        return true;
    }
    bool copper = false;
    bool tin = false;
    bool iron = false;
    for (const auto* settlement : MemberSettlements(settlements, polity)) {
        for (const int index : NearbyTileIndices(world, *settlement)) {
            const auto resource = world.Tiles()[static_cast<std::size_t>(index)].resource;
            copper = copper || resource == ResourceKind::Copper;
            tin = tin || resource == ResourceKind::Tin;
            iron = iron || resource == ResourceKind::ShallowIron || resource == ResourceKind::MeteoricIron;
        }
    }
    return (copper && tin) || iron;
}

bool IsLegalSpecialRequirement(const World& world, const std::vector<Settlement>& settlements, const Polity& polity,
                               TechId tech) {
    switch (tech) {
    case TechId::BronzeWorking:
        return HasBronzeMaterialAccess(world, settlements, polity);
    case TechId::Sailing:
        return HasCoastalAccess(world, settlements, polity);
    case TechId::None:
    case TechId::Pottery:
    case TechId::Irrigation:
    case TechId::AnimalHusbandry:
    case TechId::Mining:
    case TechId::Roads:
    case TechId::Administration:
    case TechId::Fortification:
        return true;
    }
    return false;
}

TechId FirstLegalFallback(const World& world, const std::vector<Settlement>& settlements, const Polity& polity) {
    const TechId order[] = {TechId::Pottery,        TechId::Mining,        TechId::Roads,
                            TechId::Administration, TechId::Irrigation,    TechId::AnimalHusbandry,
                            TechId::BronzeWorking,  TechId::Fortification, TechId::Sailing};
    for (const auto tech : order) {
        if (CanResearchTech(world, settlements, polity, tech)) {
            return tech;
        }
    }
    return TechId::None;
}

void StartResearchIfNeeded(const World& world, Turn turn, const std::vector<Settlement>& settlements, Polity& polity,
                           EventLog& event_log) {
    if (polity.research.current != TechId::None &&
        CanResearchTech(world, settlements, polity, polity.research.current)) {
        return;
    }
    polity.research.current = ChooseResearchFor(world, settlements, polity);
    polity.research.progress = 0.0F;
    if (polity.research.current == TechId::None) {
        return;
    }

    event_log.Add(SimEvent{
        .turn = turn,
        .type = EventType::ResearchStarted,
        .actor_id = polity.capital_settlement_id,
        .x = 0,
        .y = 0,
        .summary = polity.name + " started research: " + ToString(polity.research.current),
    });
}

void RefreshDerivedPolityTechStats(Polity& polity) {
    const auto effects = ComputeTechEffects(polity.research);
    polity.tool_efficiency = effects.bronze_working_enabled ? 1.10F : 1.0F;
    polity.military_potential = static_cast<float>(polity.population) * 0.035F;
    if (effects.bronze_working_enabled) {
        polity.military_potential *= 1.25F;
    }
    if (effects.fortification_enabled) {
        polity.military_potential *= 1.10F;
    }
}

} // namespace

float KnowledgeIncomeFor(const World& world, const std::vector<Settlement>& settlements, const Polity& polity) {
    const auto* capital = SettlementById(settlements, polity.capital_settlement_id);
    const float capital_population = capital == nullptr ? 0.0F : static_cast<float>(capital->population);
    float village_count = 0.0F;
    for (const auto* settlement : MemberSettlements(settlements, polity)) {
        village_count += settlement->level == SettlementLevel::Village ? 1.0F : 0.0F;
    }

    float income =
        capital_population * 0.006F + village_count * 0.35F + std::max(0.0F, polity.budget.wealth_surplus) * 0.03F;
    income *= std::clamp(polity.stability, 0.25F, 1.25F);
    income *= 1.0F / (1.0F + polity.overextension * 0.50F);
    if (HasCoastalAccess(world, settlements, polity)) {
        income += 0.10F;
    }
    return std::max(0.05F, income);
}

bool CanResearchTech(const World& world, const std::vector<Settlement>& settlements, const Polity& polity,
                     TechId tech) {
    if (tech == TechId::None || HasTech(polity.research, tech)) {
        return false;
    }
    const auto* def = FindTechDef(tech);
    if (def == nullptr || !PrerequisitesMet(polity.research, *def)) {
        return false;
    }
    return IsLegalSpecialRequirement(world, settlements, polity, tech);
}

TechId ChooseResearchFor(const World& world, const std::vector<Settlement>& settlements, const Polity& polity) {
    auto legal = [&](TechId tech) { return CanResearchTech(world, settlements, polity, tech); };

    if (FoodPressureHigh(settlements, polity)) {
        if (legal(TechId::Pottery)) {
            return TechId::Pottery;
        }
        if (legal(TechId::Irrigation)) {
            return TechId::Irrigation;
        }
    }
    if (polity.overextension > 0.22F) {
        if (legal(TechId::Administration)) {
            return TechId::Administration;
        }
        if (legal(TechId::Roads)) {
            return TechId::Roads;
        }
    }
    if (MinePotential(world, settlements, polity) > 0 && legal(TechId::Mining)) {
        return TechId::Mining;
    }
    if (RiverFarmPotential(world, settlements, polity) >= 2 && legal(TechId::Irrigation)) {
        return TechId::Irrigation;
    }
    if (PastureOrHorsePotential(world, settlements, polity) >= 2 && legal(TechId::AnimalHusbandry)) {
        return TechId::AnimalHusbandry;
    }
    if (legal(TechId::BronzeWorking)) {
        return TechId::BronzeWorking;
    }
    if (polity.contested_tile_count >= 10 && legal(TechId::Fortification)) {
        return TechId::Fortification;
    }
    if (HasCoastalAccess(world, settlements, polity) && legal(TechId::Sailing)) {
        return TechId::Sailing;
    }
    return FirstLegalFallback(world, settlements, polity);
}

void TechnologySystem::UpdateTechnologies(const World& world, Turn turn, const std::vector<Settlement>& settlements,
                                          std::vector<Polity>& polities, EventLog& event_log) {
    for (auto& polity : polities) {
        polity.knowledge_income = KnowledgeIncomeFor(world, settlements, polity);
        RefreshDerivedPolityTechStats(polity);

        StartResearchIfNeeded(world, turn, settlements, polity, event_log);
        if (polity.research.current == TechId::None) {
            continue;
        }

        polity.research.progress += polity.knowledge_income;
        const float cost = TechCost(polity.research.current);
        if (cost <= 0.0F || polity.research.progress < cost) {
            continue;
        }

        const TechId unlocked = polity.research.current;
        if (!HasTech(polity.research, unlocked)) {
            polity.research.unlocked.push_back(unlocked);
            if (polity.research.first_unlock_turn < 0) {
                polity.research.first_unlock_turn = static_cast<int>(turn);
            }
            event_log.Add(SimEvent{
                .turn = turn,
                .type = EventType::TechnologyUnlocked,
                .actor_id = polity.capital_settlement_id,
                .x = 0,
                .y = 0,
                .summary = polity.name + " unlocked " + ToString(unlocked),
            });
        }
        polity.research.current = TechId::None;
        polity.research.progress = 0.0F;
        RefreshDerivedPolityTechStats(polity);
    }
}

} // namespace oikumene
