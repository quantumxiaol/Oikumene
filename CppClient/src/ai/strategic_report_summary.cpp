#include "strategic_report_summary.hpp"

#include <algorithm>

#include "oikumene/sim/technology_system.hpp"
#include "strategic_report_helpers.hpp"

namespace oikumene::strategic_report {

nlohmann::json TechListToJson(const std::vector<TechId>& techs) {
    nlohmann::json value = nlohmann::json::array();
    for (const auto tech : techs) {
        value.push_back(ToString(tech));
    }
    return value;
}

nlohmann::json LegalTechsToJson(const Simulation& sim, const Polity& polity, int max_count) {
    nlohmann::json value = nlohmann::json::array();
    std::vector<const TechDef*> legal;
    for (const auto& def : AllTechDefs()) {
        if (CanResearchTech(sim.GetWorld(), sim.Settlements(), polity, def.id)) {
            legal.push_back(&def);
        }
    }
    std::sort(legal.begin(), legal.end(), [&](const TechDef* lhs, const TechDef* rhs) {
        const TechId preferred = ChooseResearchFor(sim.GetWorld(), sim.Settlements(), polity);
        if (lhs->id == preferred || rhs->id == preferred) {
            return lhs->id == preferred;
        }
        return lhs->cost < rhs->cost;
    });
    const int count = std::min(max_count, static_cast<int>(legal.size()));
    for (int i = 0; i < count; ++i) {
        value.push_back(nlohmann::json{
            {"id", ToString(legal[static_cast<std::size_t>(i)]->id)},
            {"cost", legal[static_cast<std::size_t>(i)]->cost},
            {"preferred",
             legal[static_cast<std::size_t>(i)]->id == ChooseResearchFor(sim.GetWorld(), sim.Settlements(), polity)},
        });
    }
    return value;
}

nlohmann::json StockpilesToJson(const Polity& polity) {
    return nlohmann::json{
        {"food", polity.food},
        {"wood", polity.wood},
        {"ore", polity.ore},
        {"wealth", polity.wealth},
    };
}

nlohmann::json BudgetToJson(const Polity& polity) {
    return nlohmann::json{
        {"food_surplus", polity.budget.food_surplus},
        {"wood_surplus", polity.budget.wood_surplus},
        {"ore_income", polity.budget.ore_income},
        {"wealth_surplus", polity.budget.wealth_surplus},
        {"food_maintenance", polity.budget.food_maintenance},
        {"wood_maintenance", polity.budget.wood_maintenance},
        {"admin_maintenance", polity.budget.admin_maintenance},
        {"control_maintenance", polity.budget.control_maintenance},
    };
}

nlohmann::json GeographyToJson(const Simulation& sim, const Polity& polity) {
    int controlled_land = 0;
    int contested = 0;
    int coastal_tiles = 0;
    int river_tiles = 0;
    int resource_tiles = 0;
    float fertility_sum = 0.0F;
    for (const auto& tile : sim.GetWorld().Tiles()) {
        if (tile.controller_polity_id != polity.id) {
            continue;
        }
        if (!tile.is_ocean && !tile.is_lake) {
            ++controlled_land;
            fertility_sum += tile.fertility;
        }
        contested += tile.is_contested ? 1 : 0;
        coastal_tiles += tile.is_coast ? 1 : 0;
        river_tiles += tile.has_river ? 1 : 0;
        resource_tiles += tile.resource != ResourceKind::None ? 1 : 0;
    }

    return nlohmann::json{
        {"controlled_land_tiles", controlled_land},
        {"contested_tiles", contested},
        {"coastal_tiles", coastal_tiles},
        {"river_tiles", river_tiles},
        {"resource_tiles", resource_tiles},
        {"average_controlled_fertility",
         controlled_land <= 0 ? 0.0F : fertility_sum / static_cast<float>(controlled_land)},
        {"connected_mines", polity.connected_mines},
        {"active_connected_mines", polity.active_connected_mines},
        {"connected_ore_income", polity.connected_ore_income},
        {"unconnected_ore_income", polity.unconnected_ore_income},
        {"route_count", static_cast<int>(polity.route_ids.size())},
        {"route_maintenance", polity.route_maintenance},
    };
}

nlohmann::json NeighborsToJson(const Simulation& sim, const Polity& polity, int max_count) {
    std::vector<const DiplomacyRelation*> relations;
    for (const auto& relation : sim.DiplomacyRelations()) {
        if (InvolvesPolity(relation, polity.id)) {
            relations.push_back(&relation);
        }
    }
    std::sort(relations.begin(), relations.end(), [&](const auto* lhs, const auto* rhs) {
        return RelationSortScore(*lhs, polity.id) > RelationSortScore(*rhs, polity.id);
    });

    nlohmann::json value = nlohmann::json::array();
    const int count = std::min(max_count, static_cast<int>(relations.size()));
    for (int i = 0; i < count; ++i) {
        const auto& relation = *relations[static_cast<std::size_t>(i)];
        const PolityId other_id = OtherPolity(relation, polity.id);
        const auto* other = PolityById(sim.Polities(), other_id);
        value.push_back(nlohmann::json{
            {"polity_id", other_id},
            {"name", other == nullptr ? "" : other->name},
            {"posture", ToString(relation.posture)},
            {"has_active_trade", relation.has_active_trade},
            {"trade_profit", relation.trade_profit},
            {"border_tension", relation.border_tension},
            {"friendship", relation.friendship},
            {"competition", relation.competition},
            {"dependence_on_other",
             DirectedValue(relation.dependence_a_on_b, relation.dependence_b_on_a, relation, polity.id)},
            {"other_dependence",
             DirectedValue(relation.dependence_a_on_b, relation.dependence_b_on_a, relation, other_id)},
            {"grievance_toward_other",
             DirectedValue(relation.grievance_a_to_b, relation.grievance_b_to_a, relation, polity.id)},
            {"restraint_toward_other",
             DirectedValue(relation.restraint_a_to_b, relation.restraint_b_to_a, relation, polity.id)},
            {"active_vassal_treaty_id", relation.active_vassal_treaty_id},
            {"treaty_subject_polity_id", relation.treaty_subject_polity_id},
            {"treaty_overlord_polity_id", relation.treaty_overlord_polity_id},
            {"treaty_liberty_desire", relation.treaty_liberty_desire},
        });
    }
    return value;
}

nlohmann::json WarPressuresToJson(const Simulation& sim, const Polity& polity, int max_count) {
    std::vector<const WarPressure*> pressures;
    for (const auto& pressure : sim.WarPressures()) {
        if (pressure.actor_polity_id == polity.id) {
            pressures.push_back(&pressure);
        }
    }
    std::sort(pressures.begin(), pressures.end(),
              [](const auto* lhs, const auto* rhs) { return lhs->declaration_pressure > rhs->declaration_pressure; });

    nlohmann::json value = nlohmann::json::array();
    const int count = std::min(max_count, static_cast<int>(pressures.size()));
    for (int i = 0; i < count; ++i) {
        const auto& pressure = *pressures[static_cast<std::size_t>(i)];
        value.push_back(nlohmann::json{
            {"target_polity_id", pressure.target_polity_id},
            {"objective", ToString(pressure.objective)},
            {"war_roi", pressure.war_roi},
            {"declaration_pressure", pressure.declaration_pressure},
            {"trade_conflict_weight", pressure.trade_conflict_weight},
            {"grievance_pressure", pressure.grievance_pressure},
            {"restraint_pressure", pressure.restraint_pressure},
            {"vassalage_pressure", pressure.vassalage_pressure},
        });
    }
    return value;
}

nlohmann::json WarTargetsToJson(const Simulation& sim, const Polity& polity, int max_count) {
    std::vector<const WarTargetCandidate*> targets;
    for (const auto& target : sim.WarTargets()) {
        if (target.actor_polity_id == polity.id) {
            targets.push_back(&target);
        }
    }
    std::sort(targets.begin(), targets.end(),
              [](const auto* lhs, const auto* rhs) { return lhs->action_score > rhs->action_score; });

    nlohmann::json value = nlohmann::json::array();
    const int count = std::min(max_count, static_cast<int>(targets.size()));
    for (int i = 0; i < count; ++i) {
        const auto& target = *targets[static_cast<std::size_t>(i)];
        value.push_back(nlohmann::json{
            {"id", target.id},
            {"target_polity_id", target.target_polity_id},
            {"kind", ToString(target.kind)},
            {"objective", ToString(target.objective)},
            {"x", target.x},
            {"y", target.y},
            {"roi", target.roi},
            {"action_score", target.action_score},
            {"target_value", target.target_value},
            {"campaign_cost", target.campaign_cost},
            {"occupation_cost", target.occupation_cost},
        });
    }
    return value;
}

nlohmann::json WarsToJson(const Simulation& sim, const Polity& polity, int max_count) {
    std::vector<const WarCampaign*> wars;
    for (const auto& war : sim.Wars()) {
        if (war.actor_polity_id == polity.id || war.target_polity_id == polity.id) {
            wars.push_back(&war);
        }
    }
    std::sort(wars.begin(), wars.end(), [](const auto* lhs, const auto* rhs) {
        if (lhs->status != rhs->status) {
            return lhs->status == WarCampaignStatus::Active;
        }
        return lhs->started_turn > rhs->started_turn;
    });

    nlohmann::json value = nlohmann::json::array();
    const int count = std::min(max_count, static_cast<int>(wars.size()));
    for (int i = 0; i < count; ++i) {
        const auto& war = *wars[static_cast<std::size_t>(i)];
        value.push_back(nlohmann::json{
            {"id", war.id},
            {"actor_polity_id", war.actor_polity_id},
            {"target_polity_id", war.target_polity_id},
            {"status", ToString(war.status)},
            {"objective", ToString(war.objective)},
            {"progress", war.progress},
            {"population_lost", war.population_lost},
            {"food_spent", war.food_spent},
            {"equipment_spent", war.equipment_spent},
        });
    }
    return value;
}

nlohmann::json OccupationsToJson(const Simulation& sim, const Polity& polity, int max_count) {
    std::vector<const OccupationRecord*> occupations;
    for (const auto& occupation : sim.Occupations()) {
        if (occupation.occupier_polity_id == polity.id || occupation.previous_owner_polity_id == polity.id ||
            occupation.subject_polity_id == polity.id) {
            occupations.push_back(&occupation);
        }
    }
    std::sort(occupations.begin(), occupations.end(), [](const auto* lhs, const auto* rhs) {
        if (lhs->status != rhs->status) {
            return lhs->status == OccupationStatus::Active;
        }
        return lhs->started_turn > rhs->started_turn;
    });

    nlohmann::json value = nlohmann::json::array();
    const int count = std::min(max_count, static_cast<int>(occupations.size()));
    for (int i = 0; i < count; ++i) {
        const auto& occupation = *occupations[static_cast<std::size_t>(i)];
        value.push_back(nlohmann::json{
            {"id", occupation.id},
            {"status", ToString(occupation.status)},
            {"occupier_polity_id", occupation.occupier_polity_id},
            {"previous_owner_polity_id", occupation.previous_owner_polity_id},
            {"subject_polity_id", occupation.subject_polity_id},
            {"vassal_treaty_id", occupation.vassal_treaty_id},
            {"unrest", occupation.unrest},
            {"integration", occupation.integration},
            {"revolt_risk", occupation.revolt_risk},
            {"maintenance_cost", occupation.maintenance_cost},
        });
    }
    return value;
}

nlohmann::json VassalTreatiesToJson(const Simulation& sim, const Polity& polity, int max_count) {
    std::vector<const VassalTreaty*> treaties;
    for (const auto& treaty : sim.VassalTreaties()) {
        if (treaty.overlord_polity_id == polity.id || treaty.subject_polity_id == polity.id) {
            treaties.push_back(&treaty);
        }
    }
    std::sort(treaties.begin(), treaties.end(), [](const auto* lhs, const auto* rhs) {
        if (lhs->status != rhs->status) {
            return lhs->status == VassalTreatyStatus::Active;
        }
        return lhs->started_turn > rhs->started_turn;
    });

    nlohmann::json value = nlohmann::json::array();
    const int count = std::min(max_count, static_cast<int>(treaties.size()));
    for (int i = 0; i < count; ++i) {
        const auto& treaty = *treaties[static_cast<std::size_t>(i)];
        value.push_back(nlohmann::json{
            {"id", treaty.id},
            {"status", ToString(treaty.status)},
            {"overlord_polity_id", treaty.overlord_polity_id},
            {"subject_polity_id", treaty.subject_polity_id},
            {"strength", treaty.strength},
            {"autonomy", treaty.autonomy},
            {"tribute_rate", treaty.tribute_rate},
            {"tribute_due", treaty.tribute_due},
            {"loyalty", treaty.loyalty},
            {"liberty_desire", treaty.liberty_desire},
        });
    }
    return value;
}

nlohmann::json RecentEventsToJson(const Simulation& sim, int max_count) {
    nlohmann::json value = nlohmann::json::array();
    const auto& events = sim.Events().Events();
    int emitted = 0;
    for (auto it = events.rbegin(); it != events.rend() && emitted < max_count; ++it) {
        value.push_back(nlohmann::json{
            {"turn", it->turn},
            {"type", ToString(it->type)},
            {"actor_id", it->actor_id},
            {"x", it->x},
            {"y", it->y},
            {"summary", it->summary},
        });
        ++emitted;
    }
    return value;
}
nlohmann::json StrategicSummaryToJson(const Simulation& sim, const Polity& polity,
                                      const StrategicReportOptions& options) {
    const auto* capital = SettlementById(sim.Settlements(), polity.capital_settlement_id);
    return nlohmann::json{
        {"version", "0.1"},
        {"turn", sim.CurrentTurn()},
        {"polity",
         nlohmann::json{
             {"id", polity.id},
             {"name", polity.name},
             {"level", ToString(polity.level)},
             {"capital_settlement_id", polity.capital_settlement_id},
             {"capital_population", capital == nullptr ? 0 : capital->population},
             {"population", polity.population},
             {"settlement_count", polity.member_settlement_ids.size()},
         }},
        {"stockpiles", StockpilesToJson(polity)},
        {"budget", BudgetToJson(polity)},
        {"governance",
         nlohmann::json{
             {"stability", polity.stability},
             {"legitimacy", polity.legitimacy},
             {"admin_load", polity.admin_load},
             {"admin_capacity", polity.admin_capacity},
             {"overextension", polity.overextension},
             {"control_power", polity.control_power},
             {"controlled_tiles", polity.controlled_tile_count},
             {"contested_tiles", polity.contested_tile_count},
         }},
        {"technology",
         nlohmann::json{
             {"current", ToString(polity.research.current)},
             {"progress", polity.research.progress},
             {"current_cost", TechCost(polity.research.current)},
             {"knowledge_income", polity.knowledge_income},
             {"unlocked", TechListToJson(polity.research.unlocked)},
             {"legal_options", LegalTechsToJson(sim, polity, 5)},
         }},
        {"geography", GeographyToJson(sim, polity)},
        {"trade",
         nlohmann::json{
             {"active_trade_count", polity.active_trade_count},
             {"profit", polity.trade_profit},
             {"route_cost", polity.trade_route_cost},
             {"route_efficiency", polity.trade_route_efficiency},
             {"complementarity", polity.trade_complementarity},
         }},
        {"occupation",
         nlohmann::json{
             {"load", polity.occupation_load},
             {"unrest", polity.occupation_unrest},
             {"occupied_settlements", polity.occupied_settlements},
         }},
        {"vassalage",
         nlohmann::json{
             {"overlord_polity_id", polity.overlord_polity_id},
             {"active_overlord_treaty_id", polity.active_overlord_treaty_id},
             {"vassal_count", polity.vassal_count},
             {"tribute_income", polity.vassal_tribute_income},
             {"tribute_paid", polity.vassal_tribute_paid},
             {"liberty_desire", polity.vassal_liberty_desire},
             {"overlord_protection", polity.overlord_protection},
         }},
        {"neighbors", NeighborsToJson(sim, polity, options.max_neighbors)},
        {"war_pressures", WarPressuresToJson(sim, polity, options.max_war_pressures)},
        {"war_targets", WarTargetsToJson(sim, polity, options.max_war_targets)},
        {"wars", WarsToJson(sim, polity, options.max_wars)},
        {"occupations", OccupationsToJson(sim, polity, options.max_occupations)},
        {"vassal_treaties", VassalTreatiesToJson(sim, polity, options.max_vassal_treaties)},
        {"recent_events", RecentEventsToJson(sim, options.max_recent_events)},
    };
}

} // namespace oikumene::strategic_report
