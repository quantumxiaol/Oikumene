#pragma once

#include <string>
#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/trade.hpp"
#include "oikumene/world/world.hpp"

namespace oikumene {

struct TradeCandidate {
    PolityId polity_a_id = kInvalidPolityId;
    PolityId polity_b_id = kInvalidPolityId;
    TradeGood export_from_a = TradeGood::None;
    TradeGood export_from_b = TradeGood::None;
    float value_a_to_b = 0.0F;
    float value_b_to_a = 0.0F;
    float complementarity = 0.0F;
    float route_cost = 0.0F;
    float route_cost_without_network = 0.0F;
    float route_saving = 0.0F;
    float route_efficiency = 0.0F;
    float gross_value = 0.0F;
    float transport_cost = 0.0F;
    float expected_profit = 0.0F;
    std::vector<TileCoord> path;
    bool viable = false;
    std::string reason;
};

[[nodiscard]] bool IsTradeCandidateProfitable(const TradeCandidate& candidate);
[[nodiscard]] TradeCandidate BuildTradeCandidate(const World& world, const std::vector<Settlement>& settlements,
                                                 const Polity& polity_a, const Polity& polity_b);
[[nodiscard]] std::vector<TradeCandidate> BuildTradeCandidates(const World& world,
                                                               const std::vector<Settlement>& settlements,
                                                               const std::vector<Polity>& polities);

} // namespace oikumene
