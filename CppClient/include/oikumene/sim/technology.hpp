#pragma once

#include <string>
#include <vector>

namespace oikumene {

enum class TechId {
    None,
    Pottery,
    Irrigation,
    AnimalHusbandry,
    Mining,
    Roads,
    Administration,
    BronzeWorking,
    Fortification,
    Sailing,
};

struct TechDef {
    TechId id = TechId::None;
    std::string name;
    std::string description;
    float cost = 0.0F;
    std::vector<TechId> prerequisites;
};

struct ResearchState {
    TechId current = TechId::None;
    float progress = 0.0F;
    std::vector<TechId> unlocked;
    int first_unlock_turn = -1;
};

[[nodiscard]] std::string ToString(TechId tech);
[[nodiscard]] const std::vector<TechDef>& AllTechDefs();
[[nodiscard]] const TechDef* FindTechDef(TechId tech);
[[nodiscard]] float TechCost(TechId tech);
[[nodiscard]] bool HasTech(const ResearchState& research, TechId tech);
[[nodiscard]] bool PrerequisitesMet(const ResearchState& research, const TechDef& def);

} // namespace oikumene
