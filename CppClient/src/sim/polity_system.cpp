#include "oikumene/sim/polity_system.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "oikumene/sim/control_field.hpp"

namespace oikumene {
namespace {

constexpr float kFoundingFoodThreshold = 70.0F;
constexpr int kFoundingPopulationThreshold = 120;
constexpr float kCapitalSeparationCost = 22.0F;

PolityId NextPolityId(const std::vector<Polity>& polities) {
    PolityId next = 0;
    for (const auto& polity : polities) {
        next = std::max(next, static_cast<PolityId>(polity.id + 1));
    }
    return next;
}

Settlement* SettlementById(std::vector<Settlement>& settlements, int id) {
    for (auto& settlement : settlements) {
        if (settlement.id == id) {
            return &settlement;
        }
    }
    return nullptr;
}

const Settlement* SettlementById(const std::vector<Settlement>& settlements, int id) {
    for (const auto& settlement : settlements) {
        if (settlement.id == id) {
            return &settlement;
        }
    }
    return nullptr;
}

bool ContainsSettlement(const Polity& polity, int settlement_id) {
    return std::find(polity.member_settlement_ids.begin(), polity.member_settlement_ids.end(), settlement_id) !=
           polity.member_settlement_ids.end();
}

bool IsEligibleCapital(const Settlement& settlement) {
    return settlement.level == SettlementLevel::Village && settlement.population >= kFoundingPopulationThreshold &&
           settlement.stockpile.food >= kFoundingFoodThreshold;
}

bool HasNearbyCapital(const World& world, const std::vector<Settlement>& settlements, const std::vector<Polity>& polities,
                      const Settlement& candidate) {
    for (const auto& polity : polities) {
        const auto* capital = SettlementById(settlements, polity.capital_settlement_id);
        if (capital == nullptr) {
            continue;
        }
        const float cost = TerrainPathCost(world, candidate.x, candidate.y, capital->x, capital->y, kCapitalSeparationCost);
        if (cost <= kCapitalSeparationCost) {
            return true;
        }
    }
    return false;
}

Polity* PolityById(std::vector<Polity>& polities, PolityId id) {
    for (auto& polity : polities) {
        if (polity.id == id) {
            return &polity;
        }
    }
    return nullptr;
}

void RecalculatePolityAggregates(std::vector<Settlement>& settlements, std::vector<Polity>& polities) {
    for (auto& polity : polities) {
        polity.population = 0;
        polity.food = 0.0F;
        polity.wood = 0.0F;
        polity.ore = 0.0F;
        polity.wealth = 0.0F;

        for (const int settlement_id : polity.member_settlement_ids) {
            const auto* settlement = SettlementById(settlements, settlement_id);
            if (settlement == nullptr) {
                continue;
            }
            polity.population += settlement->population;
            polity.food += settlement->stockpile.food;
            polity.wood += settlement->stockpile.wood;
            polity.ore += settlement->stockpile.ore;
            polity.wealth += settlement->stockpile.wealth;
        }

        const float member_bonus = static_cast<float>(polity.member_settlement_ids.size()) * 1.2F;
        polity.admin_range = 16.0F + std::sqrt(static_cast<float>(std::max(0, polity.population))) * 0.22F + member_bonus;
        polity.control_power = 38.0F + static_cast<float>(polity.population) * 0.045F;
    }
}

void FoundEligiblePolities(World& world,
                           Turn turn,
                           std::vector<Settlement>& settlements,
                           std::vector<Polity>& polities,
                           EventLog& event_log) {
    for (auto& settlement : settlements) {
        if (settlement.polity_id != kInvalidPolityId || !IsEligibleCapital(settlement)) {
            continue;
        }
        if (HasNearbyCapital(world, settlements, polities, settlement)) {
            continue;
        }

        const PolityId id = NextPolityId(polities);
        Polity polity;
        polity.id = id;
        polity.name = "Chiefdom " + std::to_string(id);
        polity.level = PolityLevel::Chiefdom;
        polity.capital_settlement_id = settlement.id;
        polity.member_settlement_ids.push_back(settlement.id);
        polity.debug_color_index = static_cast<std::uint32_t>(id);
        polity.founded_turn = turn;
        settlement.polity_id = id;
        settlement.is_capital = true;
        polities.push_back(polity);

        std::ostringstream summary;
        summary << polity.name << " founded at settlement " << settlement.id << " (" << settlement.x << ","
                << settlement.y << "), pop " << settlement.population;
        event_log.Add(SimEvent{
            .turn = turn,
            .type = EventType::PolityFounded,
            .actor_id = settlement.id,
            .x = settlement.x,
            .y = settlement.y,
            .summary = summary.str(),
        });
    }
}

void JoinNearbySettlements(World& world,
                           Turn turn,
                           std::vector<Settlement>& settlements,
                           std::vector<Polity>& polities,
                           EventLog& event_log) {
    for (auto& settlement : settlements) {
        if (settlement.polity_id != kInvalidPolityId) {
            continue;
        }

        Polity* best_polity = nullptr;
        float best_cost = 1.0e9F;
        for (auto& polity : polities) {
            const auto* capital = SettlementById(settlements, polity.capital_settlement_id);
            if (capital == nullptr) {
                continue;
            }
            const float cost = TerrainPathCost(world, settlement.x, settlement.y, capital->x, capital->y, polity.admin_range);
            if (cost <= polity.admin_range && cost < best_cost) {
                best_cost = cost;
                best_polity = &polity;
            }
        }

        if (best_polity == nullptr) {
            continue;
        }

        settlement.polity_id = best_polity->id;
        settlement.is_capital = false;
        if (!ContainsSettlement(*best_polity, settlement.id)) {
            best_polity->member_settlement_ids.push_back(settlement.id);
        }

        std::ostringstream summary;
        summary << "Settlement " << settlement.id << " joined " << best_polity->name << ", path cost " << best_cost;
        event_log.Add(SimEvent{
            .turn = turn,
            .type = EventType::SettlementJoinedPolity,
            .actor_id = settlement.id,
            .x = settlement.x,
            .y = settlement.y,
            .summary = summary.str(),
        });
    }
}

}  // namespace

void PolitySystem::Reset(World& world, std::vector<Settlement>& settlements, std::vector<Polity>& polities) {
    polities.clear();
    for (auto& settlement : settlements) {
        settlement.polity_id = kInvalidPolityId;
        settlement.is_capital = false;
    }
    for (auto& tile : world.Tiles()) {
        tile.owner_polity_id = kInvalidPolityId;
        tile.controller_polity_id = kInvalidPolityId;
        tile.control_strength = 0.0F;
        tile.is_contested = false;
    }
}

void PolitySystem::UpdatePolities(World& world,
                                  Turn turn,
                                  std::vector<Settlement>& settlements,
                                  std::vector<Polity>& polities,
                                  EventLog& event_log) {
    FoundEligiblePolities(world, turn, settlements, polities, event_log);
    RecalculatePolityAggregates(settlements, polities);
    JoinNearbySettlements(world, turn, settlements, polities, event_log);
    RecalculatePolityAggregates(settlements, polities);

    std::vector<int> previous_counts;
    previous_counts.reserve(polities.size());
    for (const auto& polity : polities) {
        previous_counts.push_back(polity.controlled_tile_count);
    }

    const ControlFieldStats stats = RecomputeControlField(world, settlements, polities);
    if (turn > 0 && turn % 25 == 0) {
        for (std::size_t i = 0; i < polities.size(); ++i) {
            if (polities[i].controlled_tile_count > previous_counts[i]) {
                const auto* capital = SettlementById(settlements, polities[i].capital_settlement_id);
                event_log.Add(SimEvent{
                    .turn = turn,
                    .type = EventType::ControlFieldExpanded,
                    .actor_id = polities[i].capital_settlement_id,
                    .x = capital == nullptr ? 0 : capital->x,
                    .y = capital == nullptr ? 0 : capital->y,
                    .summary = polities[i].name + " controls " +
                               std::to_string(polities[i].controlled_tile_count) + " land tiles",
                });
            }
        }
        if (stats.contested_tiles > 0) {
            event_log.Add(SimEvent{
                .turn = turn,
                .type = EventType::ContestedRegionFormed,
                .actor_id = -1,
                .x = 0,
                .y = 0,
                .summary = std::to_string(stats.contested_tiles) + " contested tiles detected",
            });
        }
    }
}

}  // namespace oikumene
