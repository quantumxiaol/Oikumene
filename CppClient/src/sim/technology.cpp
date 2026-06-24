#include "oikumene/sim/technology.hpp"

#include <algorithm>

namespace oikumene {
namespace {

std::vector<TechDef> BuildTechDefs() {
    return {
        TechDef{.id = TechId::Pottery,
                .name = "Pottery",
                .description = "Improves storage and softens food shocks.",
                .cost = 90.0F,
                .prerequisites = {}},
        TechDef{.id = TechId::Irrigation,
                .name = "Irrigation",
                .description = "Improves farm output, especially along rivers.",
                .cost = 170.0F,
                .prerequisites = {TechId::Pottery}},
        TechDef{.id = TechId::AnimalHusbandry,
                .name = "Animal Husbandry",
                .description = "Improves pasture output and horse value.",
                .cost = 150.0F,
                .prerequisites = {}},
        TechDef{.id = TechId::Mining,
                .name = "Mining",
                .description = "Turns shallow mineral sites into real ore income.",
                .cost = 150.0F,
                .prerequisites = {}},
        TechDef{.id = TechId::Roads,
                .name = "Roads",
                .description = "Reduces control path cost and distance administration load.",
                .cost = 220.0F,
                .prerequisites = {TechId::Pottery}},
        TechDef{.id = TechId::Administration,
                .name = "Administration",
                .description = "Raises administrative capacity and reduces overextension pressure.",
                .cost = 320.0F,
                .prerequisites = {TechId::Roads}},
        TechDef{.id = TechId::BronzeWorking,
                .name = "Bronze Working",
                .description = "Improves tools and future military equipment when metals exist.",
                .cost = 340.0F,
                .prerequisites = {TechId::Mining}},
        TechDef{.id = TechId::Fortification,
                .name = "Fortification",
                .description = "Improves defensive potential and reduces contested-border instability.",
                .cost = 260.0F,
                .prerequisites = {TechId::Pottery}},
        TechDef{.id = TechId::Sailing,
                .name = "Sailing",
                .description = "Prepares coastal trade and improves coastal control.",
                .cost = 230.0F,
                .prerequisites = {}},
    };
}

} // namespace

std::string ToString(TechId tech) {
    switch (tech) {
    case TechId::None:
        return "None";
    case TechId::Pottery:
        return "Pottery";
    case TechId::Irrigation:
        return "Irrigation";
    case TechId::AnimalHusbandry:
        return "AnimalHusbandry";
    case TechId::Mining:
        return "Mining";
    case TechId::Roads:
        return "Roads";
    case TechId::Administration:
        return "Administration";
    case TechId::BronzeWorking:
        return "BronzeWorking";
    case TechId::Fortification:
        return "Fortification";
    case TechId::Sailing:
        return "Sailing";
    }
    return "Unknown";
}

const std::vector<TechDef>& AllTechDefs() {
    static const std::vector<TechDef> defs = BuildTechDefs();
    return defs;
}

const TechDef* FindTechDef(TechId tech) {
    const auto& defs = AllTechDefs();
    const auto it = std::find_if(defs.begin(), defs.end(), [&](const TechDef& def) { return def.id == tech; });
    return it == defs.end() ? nullptr : &(*it);
}

float TechCost(TechId tech) {
    const auto* def = FindTechDef(tech);
    return def == nullptr ? 0.0F : def->cost;
}

bool HasTech(const ResearchState& research, TechId tech) {
    if (tech == TechId::None) {
        return true;
    }
    return std::find(research.unlocked.begin(), research.unlocked.end(), tech) != research.unlocked.end();
}

bool PrerequisitesMet(const ResearchState& research, const TechDef& def) {
    return std::all_of(def.prerequisites.begin(), def.prerequisites.end(),
                       [&](TechId prerequisite) { return HasTech(research, prerequisite); });
}

} // namespace oikumene
