#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <raylib.h>

namespace oikumene {

enum class SymbolCategory {
    Biome,
    Water,
    Resource,
    Improvement,
    Route,
    Entity,
    Polity,
    Overlay,
};

struct SymbolInfo {
    SymbolCategory category = SymbolCategory::Overlay;
    std::string id;
    std::string label;
    std::string description;
    Color color{255, 255, 255, 255};
    std::string shape = "square";
    bool visible_in_legend = true;
};

[[nodiscard]] std::string ToString(SymbolCategory category);
[[nodiscard]] const std::vector<SymbolInfo>& LegendSymbols();
[[nodiscard]] std::vector<SymbolInfo> LegendSymbolsFor(SymbolCategory category);
[[nodiscard]] const SymbolInfo* FindSymbolById(const std::string& id);

}  // namespace oikumene
