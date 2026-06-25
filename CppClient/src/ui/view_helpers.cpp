#include "oikumene/ui/view_helpers.hpp"

#include <algorithm>
#include <sstream>

#include "oikumene/sim/settlement.hpp"
#include "oikumene/sim/technology.hpp"

namespace oikumene {

Color StatusColor(bool online) {
    return online ? Color{80, 210, 120, 255} : Color{230, 90, 75, 255};
}

std::string StatusText(const HealthStatus& status) {
    if (status.online) {
        return "online (" + std::to_string(status.latency_ms) + " ms)";
    }
    if (!status.message.empty()) {
        return "offline - " + status.message;
    }
    return "offline";
}

std::string Fixed(float value, int precision) {
    std::ostringstream stream;
    stream.setf(std::ios::fixed);
    stream.precision(precision);
    stream << value;
    return stream.str();
}

std::string Truncate(std::string text, std::size_t limit) {
    if (text.size() <= limit) {
        return text;
    }
    return text.substr(0, limit > 3 ? limit - 3 : limit) + "...";
}

int ActiveBandCount(const std::vector<Band>& bands) {
    int count = 0;
    for (const auto& band : bands) {
        count += band.active ? 1 : 0;
    }
    return count;
}

int VillageCount(const std::vector<Settlement>& settlements) {
    int count = 0;
    for (const auto& settlement : settlements) {
        count += settlement.level == SettlementLevel::Village ? 1 : 0;
    }
    return count;
}

int TotalPopulation(const std::vector<Band>& bands, const std::vector<Settlement>& settlements) {
    int total = 0;
    for (const auto& band : bands) {
        if (band.active) {
            total += band.population;
        }
    }
    for (const auto& settlement : settlements) {
        total += settlement.population;
    }
    return total;
}

const Settlement* SettlementById(const std::vector<Settlement>& settlements, int id) {
    for (const auto& settlement : settlements) {
        if (settlement.id == id) {
            return &settlement;
        }
    }
    return nullptr;
}

const Settlement* SettlementAt(const std::vector<Settlement>& settlements, int x, int y) {
    for (const auto& settlement : settlements) {
        if (settlement.x == x && settlement.y == y) {
            return &settlement;
        }
    }
    return nullptr;
}

const Band* BandById(const std::vector<Band>& bands, int id) {
    for (const auto& band : bands) {
        if (band.id == id) {
            return &band;
        }
    }
    return nullptr;
}

const Band* ActiveBandAt(const std::vector<Band>& bands, int x, int y) {
    for (const auto& band : bands) {
        if (band.active && band.x == x && band.y == y) {
            return &band;
        }
    }
    return nullptr;
}

const Polity* PolityById(const std::vector<Polity>& polities, PolityId id) {
    for (const auto& polity : polities) {
        if (polity.id == id) {
            return &polity;
        }
    }
    return nullptr;
}

const Route* RouteById(const std::vector<Route>& routes, int id) {
    for (const auto& route : routes) {
        if (route.id == id) {
            return &route;
        }
    }
    return nullptr;
}

const DiplomacyRelation* DiplomacyRelationForPair(const std::vector<DiplomacyRelation>& relations, PolityId a,
                                                  PolityId b) {
    for (const auto& relation : relations) {
        const bool same_pair = (relation.polity_a_id == a && relation.polity_b_id == b) ||
                               (relation.polity_a_id == b && relation.polity_b_id == a);
        if (same_pair) {
            return &relation;
        }
    }
    return nullptr;
}

std::vector<const TradeAgreement*> ActiveTradesAtTile(const std::vector<TradeAgreement>& trades, int x, int y) {
    std::vector<const TradeAgreement*> matches;
    for (const auto& trade : trades) {
        if (!trade.active) {
            continue;
        }
        const bool contains = std::any_of(trade.path.begin(), trade.path.end(),
                                          [&](const TileCoord& coord) { return coord.x == x && coord.y == y; });
        if (contains) {
            matches.push_back(&trade);
        }
    }
    return matches;
}

std::string TradeAgreementLine(const TradeAgreement& trade) {
    return "Trade " + std::to_string(trade.id) + " P" + std::to_string(trade.polity_a_id) + "-P" +
           std::to_string(trade.polity_b_id) + " " + ToString(trade.export_from_a) + "/" +
           ToString(trade.export_from_b) + " profit " + Fixed(trade.expected_profit, 2) + " eff " +
           Fixed(trade.route_efficiency, 2) + " weak " + std::to_string(trade.weak_refresh_count);
}

std::string DiplomacyRelationLine(const DiplomacyRelation& relation) {
    return "P" + std::to_string(relation.polity_a_id) + "-P" + std::to_string(relation.polity_b_id) + " " +
           ToString(relation.posture) + " F " + Fixed(relation.friendship, 2) + " C " + Fixed(relation.competition, 2) +
           " D " + Fixed(std::max(relation.dependence_a_on_b, relation.dependence_b_on_a), 2) + " B " +
           Fixed(relation.blockade_tendency, 2) + " G " +
           Fixed(std::max(relation.grievance_a_to_b, relation.grievance_b_to_a), 2) + " V " +
           Fixed(std::max(relation.vassalage_a_to_b, relation.vassalage_b_to_a), 2) + " R " +
           Fixed(std::max(relation.restraint_a_to_b, relation.restraint_b_to_a), 2) + " T " +
           std::to_string(relation.active_vassal_treaty_id) + " Ld " + Fixed(relation.treaty_liberty_desire, 2);
}

std::string TradeIdsForPolity(const std::vector<TradeAgreement>& trades, PolityId polity_id) {
    std::string text;
    for (const auto& trade : trades) {
        if (!trade.active || (trade.polity_a_id != polity_id && trade.polity_b_id != polity_id)) {
            continue;
        }
        if (!text.empty()) {
            text += ", ";
        }
        text += "#" + std::to_string(trade.id);
    }
    return text.empty() ? "None" : text;
}

int DiplomacyPostureCount(const std::vector<DiplomacyRelation>& relations, DiplomaticPosture posture) {
    int count = 0;
    for (const auto& relation : relations) {
        count += relation.posture == posture ? 1 : 0;
    }
    return count;
}

std::vector<const DiplomacyRelation*> DiplomacyRelationsForPolity(const std::vector<DiplomacyRelation>& relations,
                                                                  PolityId polity_id) {
    std::vector<const DiplomacyRelation*> matches;
    for (const auto& relation : relations) {
        if (relation.polity_a_id == polity_id || relation.polity_b_id == polity_id) {
            matches.push_back(&relation);
        }
    }
    std::sort(matches.begin(), matches.end(), [](const auto* lhs, const auto* rhs) {
        return lhs->blockade_tendency + lhs->competition > rhs->blockade_tendency + rhs->competition;
    });
    return matches;
}

std::string WarPressureLine(const WarPressure& pressure) {
    return "P" + std::to_string(pressure.actor_polity_id) + "->P" + std::to_string(pressure.target_polity_id) + " " +
           ToString(pressure.objective) + " ROI " + Fixed(pressure.war_roi, 2) + " Press " +
           Fixed(pressure.declaration_pressure, 2) + " Fpen " + Fixed(pressure.friendly_penalty, 2) + " TradeW " +
           Fixed(pressure.trade_conflict_weight, 2) + " G " + Fixed(pressure.grievance_pressure, 2) + " R " +
           Fixed(pressure.restraint_pressure, 2);
}

std::vector<const WarPressure*> WarPressuresForActor(const std::vector<WarPressure>& pressures, PolityId polity_id) {
    std::vector<const WarPressure*> matches;
    for (const auto& pressure : pressures) {
        if (pressure.actor_polity_id == polity_id) {
            matches.push_back(&pressure);
        }
    }
    std::sort(matches.begin(), matches.end(),
              [](const auto* lhs, const auto* rhs) { return lhs->declaration_pressure > rhs->declaration_pressure; });
    return matches;
}

std::string WarTargetLine(const WarTargetCandidate& target) {
    return "P" + std::to_string(target.actor_polity_id) + "->P" + std::to_string(target.target_polity_id) + " " +
           ToString(target.kind) + " " + ToString(target.objective) + " ROI " + Fixed(target.roi, 2) + " Score " +
           Fixed(target.action_score, 2) + " Cost " + Fixed(target.campaign_cost, 2);
}

std::vector<const WarTargetCandidate*> WarTargetsForActor(const std::vector<WarTargetCandidate>& targets,
                                                          PolityId polity_id) {
    std::vector<const WarTargetCandidate*> matches;
    for (const auto& target : targets) {
        if (target.actor_polity_id == polity_id) {
            matches.push_back(&target);
        }
    }
    std::sort(matches.begin(), matches.end(),
              [](const auto* lhs, const auto* rhs) { return lhs->action_score > rhs->action_score; });
    return matches;
}

std::string WarCampaignLine(const WarCampaign& campaign) {
    return "War " + std::to_string(campaign.id) + " P" + std::to_string(campaign.actor_polity_id) + "->P" +
           std::to_string(campaign.target_polity_id) + " " + ToString(campaign.status) + " " +
           ToString(campaign.target_kind) + " Prog " + Fixed(campaign.progress, 2) + " Loss " +
           Fixed(campaign.population_lost, 0);
}

std::vector<const WarCampaign*> WarsForPolity(const std::vector<WarCampaign>& campaigns, PolityId polity_id) {
    std::vector<const WarCampaign*> matches;
    for (const auto& campaign : campaigns) {
        if (campaign.actor_polity_id == polity_id || campaign.target_polity_id == polity_id) {
            matches.push_back(&campaign);
        }
    }
    std::sort(matches.begin(), matches.end(), [](const auto* lhs, const auto* rhs) {
        if (lhs->status != rhs->status) {
            return lhs->status == WarCampaignStatus::Active;
        }
        return lhs->started_turn > rhs->started_turn;
    });
    return matches;
}

std::string OccupationLine(const OccupationRecord& occupation) {
    return "Occ " + std::to_string(occupation.id) + " P" + std::to_string(occupation.occupier_polity_id) + "<-P" +
           std::to_string(occupation.previous_owner_polity_id) + " " + ToString(occupation.status) + " U " +
           Fixed(occupation.unrest, 2) + " Int " + Fixed(occupation.integration, 2) + " M " +
           Fixed(occupation.maintenance_cost, 2);
}

std::vector<const OccupationRecord*> OccupationsForPolity(const std::vector<OccupationRecord>& occupations,
                                                          PolityId polity_id) {
    std::vector<const OccupationRecord*> matches;
    for (const auto& occupation : occupations) {
        if (occupation.occupier_polity_id == polity_id || occupation.previous_owner_polity_id == polity_id ||
            occupation.subject_polity_id == polity_id) {
            matches.push_back(&occupation);
        }
    }
    std::sort(matches.begin(), matches.end(), [](const auto* lhs, const auto* rhs) {
        if (lhs->status != rhs->status) {
            return lhs->status == OccupationStatus::Active;
        }
        return lhs->started_turn > rhs->started_turn;
    });
    return matches;
}

std::string VassalTreatyLine(const VassalTreaty& treaty) {
    return "Treaty " + std::to_string(treaty.id) + " P" + std::to_string(treaty.subject_polity_id) + " under P" +
           std::to_string(treaty.overlord_polity_id) + " " + ToString(treaty.status) + " Str " +
           Fixed(treaty.strength, 2) + " Loy " + Fixed(treaty.loyalty, 2) + " Lib " + Fixed(treaty.liberty_desire, 2) +
           " Trib " + Fixed(treaty.tribute_due, 2);
}

std::vector<const VassalTreaty*> VassalTreatiesForPolity(const std::vector<VassalTreaty>& treaties,
                                                         PolityId polity_id) {
    std::vector<const VassalTreaty*> matches;
    for (const auto& treaty : treaties) {
        if (treaty.overlord_polity_id == polity_id || treaty.subject_polity_id == polity_id) {
            matches.push_back(&treaty);
        }
    }
    std::sort(matches.begin(), matches.end(), [](const auto* lhs, const auto* rhs) {
        if (lhs->status != rhs->status) {
            return lhs->status == VassalTreatyStatus::Active;
        }
        return lhs->started_turn > rhs->started_turn;
    });
    return matches;
}

int ContestedTileCount(const World& world) {
    int count = 0;
    for (const auto& tile : world.Tiles()) {
        count += tile.is_contested ? 1 : 0;
    }
    return count;
}

int RouteTileCount(const World& world) {
    int count = 0;
    for (const auto& tile : world.Tiles()) {
        count += tile.has_route ? 1 : 0;
    }
    return count;
}

int ActiveTradeCount(const std::vector<TradeAgreement>& trades) {
    int count = 0;
    for (const auto& trade : trades) {
        count += trade.active ? 1 : 0;
    }
    return count;
}

float TotalTradeProfit(const std::vector<TradeAgreement>& trades) {
    float total = 0.0F;
    for (const auto& trade : trades) {
        if (trade.active) {
            total += trade.expected_profit;
        }
    }
    return total;
}

float ControlledLandRatio(const World& world) {
    int land = 0;
    int controlled = 0;
    for (const auto& tile : world.Tiles()) {
        if (tile.is_ocean || tile.is_lake) {
            continue;
        }
        ++land;
        controlled += tile.controller_polity_id != kInvalidPolityId ? 1 : 0;
    }
    return land <= 0 ? 0.0F : static_cast<float>(controlled) / static_cast<float>(land);
}

int LargestPolityPopulation(const std::vector<Polity>& polities) {
    int largest = 0;
    for (const auto& polity : polities) {
        largest = std::max(largest, polity.population);
    }
    return largest;
}

float AverageUnlockedTechs(const std::vector<Polity>& polities) {
    if (polities.empty()) {
        return 0.0F;
    }
    int unlocked = 0;
    for (const auto& polity : polities) {
        unlocked += static_cast<int>(polity.research.unlocked.size());
    }
    return static_cast<float>(unlocked) / static_cast<float>(polities.size());
}

float TechUnlockRate(const std::vector<Polity>& polities, TechId tech) {
    if (polities.empty()) {
        return 0.0F;
    }
    int count = 0;
    for (const auto& polity : polities) {
        count += HasTech(polity.research, tech) ? 1 : 0;
    }
    return static_cast<float>(count) / static_cast<float>(polities.size());
}

std::string TechList(const ResearchState& research, std::size_t limit) {
    if (research.unlocked.empty()) {
        return "None";
    }
    std::string text;
    const std::size_t count = std::min(limit, research.unlocked.size());
    for (std::size_t i = 0; i < count; ++i) {
        if (!text.empty()) {
            text += ", ";
        }
        text += ToString(research.unlocked[i]);
    }
    if (research.unlocked.size() > limit) {
        text += ", ...";
    }
    return text;
}

std::string EffectsSummary(const TechEffects& effects) {
    const auto descriptions = DescribeActiveEffects(effects);
    if (descriptions.empty()) {
        return "None";
    }
    std::string text;
    for (std::size_t i = 0; i < descriptions.size() && i < 4; ++i) {
        if (!text.empty()) {
            text += ", ";
        }
        text += descriptions[i];
    }
    if (descriptions.size() > 4) {
        text += ", ...";
    }
    return text;
}

} // namespace oikumene
