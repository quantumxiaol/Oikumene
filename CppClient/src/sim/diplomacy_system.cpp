#include "oikumene/sim/diplomacy_system.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>

namespace oikumene {
namespace {

float Clamp01(float value) {
    return std::clamp(value, 0.0F, 1.0F);
}

bool SamePair(const TradeAgreement& trade, PolityId a, PolityId b) {
    return (trade.polity_a_id == a && trade.polity_b_id == b) || (trade.polity_a_id == b && trade.polity_b_id == a);
}

const TradeAgreement* ActiveTradeForPair(const std::vector<TradeAgreement>& trades, PolityId a, PolityId b) {
    for (const auto& trade : trades) {
        if (trade.active && SamePair(trade, a, b)) {
            return &trade;
        }
    }
    return nullptr;
}

float ControlledBorderTension(const World& world, PolityId a, PolityId b) {
    int border_edges = 0;
    int contested_near_border = 0;
    static constexpr std::array<std::pair<int, int>, 4> kDirs{
        std::pair<int, int>{1, 0},
        std::pair<int, int>{-1, 0},
        std::pair<int, int>{0, 1},
        std::pair<int, int>{0, -1},
    };

    for (const auto& tile : world.Tiles()) {
        if (tile.controller_polity_id != a && tile.controller_polity_id != b) {
            continue;
        }
        for (const auto& [dx, dy] : kDirs) {
            const int nx = tile.x + dx;
            const int ny = tile.y + dy;
            if (!world.InBounds(nx, ny)) {
                continue;
            }
            const auto& neighbor = world.At(nx, ny);
            if ((tile.controller_polity_id == a && neighbor.controller_polity_id == b) ||
                (tile.controller_polity_id == b && neighbor.controller_polity_id == a)) {
                ++border_edges;
                contested_near_border += tile.is_contested || neighbor.is_contested ? 1 : 0;
            }
        }
    }

    return Clamp01(static_cast<float>(border_edges) / 28.0F + static_cast<float>(contested_near_border) / 42.0F);
}

std::array<float, 4> ExportVector(const Polity& polity) {
    return {
        std::max(0.0F, polity.budget.food_surplus),
        std::max(0.0F, polity.budget.wood_surplus),
        std::max(0.0F, polity.budget.ore_income),
        std::max(0.0F, polity.budget.wealth_surplus),
    };
}

float EconomicOverlap(const Polity& a, const Polity& b) {
    const auto va = ExportVector(a);
    const auto vb = ExportVector(b);
    float dot = 0.0F;
    float norm_a = 0.0F;
    float norm_b = 0.0F;
    for (std::size_t i = 0; i < va.size(); ++i) {
        dot += va[i] * vb[i];
        norm_a += va[i] * va[i];
        norm_b += vb[i] * vb[i];
    }
    if (norm_a <= 0.0001F || norm_b <= 0.0001F) {
        return 0.0F;
    }
    return Clamp01(dot / std::sqrt(norm_a * norm_b));
}

float MilitaryRivalry(const Polity& a, const Polity& b) {
    const float stronger = std::max({1.0F, a.military_potential, b.military_potential});
    return Clamp01(1.0F - std::abs(a.military_potential - b.military_potential) / stronger);
}

float TradeIntensity(const TradeAgreement* trade) {
    if (trade == nullptr) {
        return 0.0F;
    }
    return Clamp01((trade->complementarity + std::max(0.0F, trade->expected_profit)) / 4.0F);
}

float DependencyShare(float directional_value, float complementarity) {
    if (complementarity <= 0.0001F) {
        return 0.0F;
    }
    const float intensity = Clamp01(complementarity / 1.8F);
    return Clamp01((directional_value / complementarity) * intensity);
}

void FillTradeFeatures(DiplomacyRelation& relation, const TradeAgreement* trade) {
    if (trade == nullptr) {
        return;
    }
    relation.has_active_trade = true;
    relation.trade_id = trade->id;
    relation.trade_profit = trade->expected_profit;
    relation.trade_complementarity = trade->complementarity;
    relation.trade_route_efficiency = trade->route_efficiency;
    relation.trade_route_cost = trade->route_cost;

    if (trade->polity_a_id == relation.polity_a_id) {
        relation.dependence_a_on_b = DependencyShare(trade->value_b_to_a, trade->complementarity);
        relation.dependence_b_on_a = DependencyShare(trade->value_a_to_b, trade->complementarity);
    } else {
        relation.dependence_a_on_b = DependencyShare(trade->value_a_to_b, trade->complementarity);
        relation.dependence_b_on_a = DependencyShare(trade->value_b_to_a, trade->complementarity);
    }

    if (relation.dependence_a_on_b > relation.dependence_b_on_a + 0.08F) {
        relation.dependent_polity_id = relation.polity_a_id;
        relation.leverage_polity_id = relation.polity_b_id;
    } else if (relation.dependence_b_on_a > relation.dependence_a_on_b + 0.08F) {
        relation.dependent_polity_id = relation.polity_b_id;
        relation.leverage_polity_id = relation.polity_a_id;
    }
}

DiplomaticPosture ClassifyPosture(const DiplomacyRelation& relation) {
    const float max_dependence = std::max(relation.dependence_a_on_b, relation.dependence_b_on_a);
    if (relation.blockade_tendency >= 0.58F && max_dependence >= 0.35F) {
        return DiplomaticPosture::BlockadeRisk;
    }
    if (max_dependence >= 0.58F && relation.friendship >= relation.competition * 0.75F) {
        return DiplomaticPosture::Dependent;
    }
    if (relation.friendship >= 0.55F && relation.friendship >= relation.competition) {
        return DiplomaticPosture::Friendly;
    }
    if (relation.competition >= 0.45F) {
        return DiplomaticPosture::Competitive;
    }
    return DiplomaticPosture::Neutral;
}

std::string BuildReason(const DiplomacyRelation& relation) {
    std::ostringstream stream;
    stream << ToString(relation.posture) << ": trade " << relation.trade_id << " friendship " << relation.friendship
           << " competition " << relation.competition << " dependence "
           << std::max(relation.dependence_a_on_b, relation.dependence_b_on_a) << " blockade "
           << relation.blockade_tendency;
    return stream.str();
}

DiplomacyRelation BuildRelation(int id, const World& world, Turn turn, const Polity& polity_a, const Polity& polity_b,
                                const std::vector<TradeAgreement>& trades) {
    DiplomacyRelation relation;
    relation.id = id;
    relation.polity_a_id = polity_a.id;
    relation.polity_b_id = polity_b.id;

    const auto* trade = ActiveTradeForPair(trades, polity_a.id, polity_b.id);
    FillTradeFeatures(relation, trade);

    relation.border_tension = ControlledBorderTension(world, polity_a.id, polity_b.id);
    relation.economic_overlap = EconomicOverlap(polity_a, polity_b);
    const float trade_intensity = TradeIntensity(trade);
    const float duration_bonus =
        trade == nullptr ? 0.0F : std::min(0.20F, static_cast<float>(turn - trade->opened_turn) / 160.0F);
    const float weak_penalty = trade == nullptr ? 0.0F : static_cast<float>(trade->weak_refresh_count) * 0.08F;
    const float route_efficiency = trade == nullptr ? 0.0F : trade->route_efficiency;
    const float military_rivalry = MilitaryRivalry(polity_a, polity_b);

    relation.friendship = Clamp01(0.05F + trade_intensity * 0.58F + route_efficiency * 0.22F + duration_bonus -
                                  relation.border_tension * 0.25F - weak_penalty);
    relation.competition = Clamp01(relation.border_tension * 0.52F + relation.economic_overlap * 0.22F +
                                   military_rivalry * relation.border_tension * 0.20F - trade_intensity * 0.18F -
                                   relation.friendship * 0.08F);

    const float max_dependence = std::max(relation.dependence_a_on_b, relation.dependence_b_on_a);
    const float route_fragility = trade == nullptr ? 0.0F : 1.0F - route_efficiency;
    relation.blockade_tendency = Clamp01(max_dependence * 0.42F + relation.competition * 0.36F +
                                         route_fragility * 0.24F - relation.friendship * 0.18F + weak_penalty);
    relation.posture = ClassifyPosture(relation);
    relation.reason = BuildReason(relation);
    return relation;
}

} // namespace

void DiplomacySystem::Reset(std::vector<DiplomacyRelation>& relations) {
    relations.clear();
}

void DiplomacySystem::UpdateDiplomacy(const World& world, Turn turn, const std::vector<Polity>& polities,
                                      const std::vector<TradeAgreement>& trades,
                                      std::vector<DiplomacyRelation>& relations) {
    relations.clear();
    int next_id = 0;
    for (std::size_t i = 0; i < polities.size(); ++i) {
        for (std::size_t j = i + 1; j < polities.size(); ++j) {
            relations.push_back(BuildRelation(next_id++, world, turn, polities[i], polities[j], trades));
        }
    }
}

} // namespace oikumene
