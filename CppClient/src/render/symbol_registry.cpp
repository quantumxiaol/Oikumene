#include "oikumene/render/symbol_registry.hpp"

#include <algorithm>
#include <utility>

#include "oikumene/render/color_palette.hpp"
#include "oikumene/world/biome.hpp"
#include "oikumene/world/improvement.hpp"
#include "oikumene/world/resource.hpp"

namespace oikumene {
namespace {

SymbolInfo Symbol(SymbolCategory category,
                  std::string id,
                  std::string label,
                  std::string description,
                  Color color,
                  std::string shape) {
    return SymbolInfo{
        .category = category,
        .id = std::move(id),
        .label = std::move(label),
        .description = std::move(description),
        .color = color,
        .shape = std::move(shape),
        .visible_in_legend = true,
    };
}

std::vector<SymbolInfo> BuildSymbols() {
    return {
        Symbol(SymbolCategory::Biome, "biome.ocean", "Ocean", "Deep water blocks early control and settlement.",
               ColorForBiome(Biome::Ocean), "square"),
        Symbol(SymbolCategory::Biome, "biome.coast", "Coast", "Coastal land supports fishing, routes, and later ports.",
               ColorForBiome(Biome::Coast), "square"),
        Symbol(SymbolCategory::Biome, "biome.grassland", "Grassland", "Open land favors pasture, movement, and farms.",
               ColorForBiome(Biome::Grassland), "square"),
        Symbol(SymbolCategory::Biome, "biome.forest", "Forest", "Forest provides wood but slows control and movement.",
               ColorForBiome(Biome::Forest), "square"),
        Symbol(SymbolCategory::Biome, "biome.rainforest", "Rainforest", "Dense wet forest is rich but hard to govern.",
               ColorForBiome(Biome::Rainforest), "square"),
        Symbol(SymbolCategory::Biome, "biome.wetland", "Wetland", "Wet terrain is fertile but costly to cross.",
               ColorForBiome(Biome::Wetland), "square"),
        Symbol(SymbolCategory::Biome, "biome.desert", "Desert", "Dry land has low fertility and high control cost.",
               ColorForBiome(Biome::Desert), "square"),
        Symbol(SymbolCategory::Biome, "biome.tundra", "Tundra", "Cold low-yield land slows population growth.",
               ColorForBiome(Biome::Tundra), "square"),
        Symbol(SymbolCategory::Biome, "biome.snow", "Snow", "Severe cold terrain is costly to settle and cross.",
               ColorForBiome(Biome::Snow), "square"),
        Symbol(SymbolCategory::Biome, "biome.hill", "Hill", "Hills favor minerals and defense but slow expansion.",
               ColorForBiome(Biome::Hill), "square"),
        Symbol(SymbolCategory::Biome, "biome.mountain", "Mountain", "Mountains strongly resist control-field spread.",
               ColorForBiome(Biome::Mountain), "square"),

        Symbol(SymbolCategory::Water, "water.river", "River", "River overlay boosts settlement value and corridor control.",
               Color{44, 135, 225, 255}, "line"),
        Symbol(SymbolCategory::Water, "water.lake", "Lake", "Lake tiles provide water access but block land control.",
               ColorForBiome(Biome::Lake), "circle"),
        Symbol(SymbolCategory::Water, "water.coastline", "Coastline", "Coastline marks early maritime potential.",
               Color{73, 150, 184, 255}, "ring"),

        Symbol(SymbolCategory::Resource, "resource.wood", "Wood", "Construction and fuel resource from forests.",
               ColorForResource(ResourceKind::Wood), "circle"),
        Symbol(SymbolCategory::Resource, "resource.bamboo", "Bamboo", "Warm wet-region construction and tool material.",
               ColorForResource(ResourceKind::Bamboo), "circle"),
        Symbol(SymbolCategory::Resource, "resource.horse", "Horse", "Open-land animal resource for mobility and future warfare.",
               ColorForResource(ResourceKind::Horse), "triangle"),
        Symbol(SymbolCategory::Resource, "resource.copper", "Copper", "Early metal resource for bronze paths.",
               ColorForResource(ResourceKind::Copper), "circle"),
        Symbol(SymbolCategory::Resource, "resource.tin", "Tin", "Bronze partner resource, usually rarer than copper.",
               ColorForResource(ResourceKind::Tin), "circle"),
        Symbol(SymbolCategory::Resource, "resource.iron", "Shallow Iron", "Accessible iron source before deep mining.",
               ColorForResource(ResourceKind::ShallowIron), "circle"),
        Symbol(SymbolCategory::Resource, "resource.coal", "Shallow Coal", "Fuel resource for later metallurgy.",
               ColorForResource(ResourceKind::ShallowCoal), "circle"),
        Symbol(SymbolCategory::Resource, "resource.meteoric_iron", "Meteoric Iron", "Rare high-value early metal source.",
               ColorForResource(ResourceKind::MeteoricIron), "star"),
        Symbol(SymbolCategory::Resource, "resource.gold", "Gold", "Prestige and wealth resource.",
               ColorForResource(ResourceKind::Gold), "circle"),
        Symbol(SymbolCategory::Resource, "resource.silver", "Silver", "Prestige and wealth resource.",
               ColorForResource(ResourceKind::Silver), "circle"),
        Symbol(SymbolCategory::Resource, "resource.stone", "Stone", "Building and fortification material.",
               ColorForResource(ResourceKind::Stone), "square"),
        Symbol(SymbolCategory::Resource, "resource.clay", "Clay", "Pottery, brick, and storage material.",
               ColorForResource(ResourceKind::Clay), "circle"),
        Symbol(SymbolCategory::Resource, "resource.salt", "Salt", "Food preservation and trade resource.",
               ColorForResource(ResourceKind::Salt), "circle"),

        Symbol(SymbolCategory::Improvement, "improvement.farm", "Farm", "Consumes labor, raises food output and carrying capacity.",
               Color{226, 202, 104, 255}, "grid"),
        Symbol(SymbolCategory::Improvement, "improvement.lumbercamp", "Lumber Camp", "Uses nearby forest for wood production.",
               Color{128, 78, 42, 255}, "square"),
        Symbol(SymbolCategory::Improvement, "improvement.pasture", "Pasture", "Uses grassland or horse terrain for food and mobility potential.",
               Color{166, 218, 124, 255}, "ring"),
        Symbol(SymbolCategory::Improvement, "improvement.shallowmine", "Shallow Mine", "Extracts accessible ore from hills and mineral tiles.",
               Color{54, 56, 58, 255}, "ring"),
        Symbol(SymbolCategory::Improvement, "improvement.foraging", "Foraging Ground", "Early subsistence tile used before full agriculture.",
               Color{80, 190, 108, 255}, "circle"),

        Symbol(SymbolCategory::Entity, "entity.band", "Band", "Mobile hunting and gathering group before settlement.",
               Color{245, 245, 236, 255}, "triangle"),
        Symbol(SymbolCategory::Entity, "entity.camp", "Camp", "Early fixed settlement.",
               Color{238, 218, 144, 255}, "square"),
        Symbol(SymbolCategory::Entity, "entity.village", "Village", "Mature settlement that can form or join a polity.",
               Color{246, 235, 185, 255}, "house"),
        Symbol(SymbolCategory::Entity, "entity.capital", "Capital", "Polity center and strongest control source.",
               Color{255, 216, 96, 255}, "double_ring"),

        Symbol(SymbolCategory::Polity, "polity.controlled", "Controlled Tile", "Tile under the strongest polity control field.",
               Color{86, 158, 226, 160}, "square"),
        Symbol(SymbolCategory::Polity, "polity.contested", "Contested Tile", "Two control fields are close enough to cause border tension.",
               Color{246, 226, 108, 255}, "cross"),
        Symbol(SymbolCategory::Polity, "polity.capital_ring", "Capital Ring", "Capital marker on the Polity Control layer.",
               Color{255, 216, 96, 255}, "ring"),
        Symbol(SymbolCategory::Overlay, "overlay.selection", "Selection", "Current selected tile, band, or settlement.",
               Color{255, 216, 96, 255}, "outline"),
        Symbol(SymbolCategory::Overlay, "overlay.hover", "Hover", "Tile under the mouse cursor when no UI captures input.",
               Color{240, 246, 255, 255}, "outline"),
    };
}

}  // namespace

std::string ToString(SymbolCategory category) {
    switch (category) {
        case SymbolCategory::Biome:
            return "Terrain / 地形";
        case SymbolCategory::Water:
            return "Water / 水文";
        case SymbolCategory::Resource:
            return "Resources / 资源";
        case SymbolCategory::Improvement:
            return "Improvements / 改良";
        case SymbolCategory::Entity:
            return "Entities / 聚落";
        case SymbolCategory::Polity:
            return "Polity / 政治";
        case SymbolCategory::Overlay:
            return "Overlay / 覆盖层";
    }
    return "Unknown";
}

const std::vector<SymbolInfo>& LegendSymbols() {
    static const std::vector<SymbolInfo> symbols = BuildSymbols();
    return symbols;
}

std::vector<SymbolInfo> LegendSymbolsFor(SymbolCategory category) {
    std::vector<SymbolInfo> result;
    for (const auto& symbol : LegendSymbols()) {
        if (symbol.category == category && symbol.visible_in_legend) {
            result.push_back(symbol);
        }
    }
    return result;
}

const SymbolInfo* FindSymbolById(const std::string& id) {
    const auto& symbols = LegendSymbols();
    const auto it = std::find_if(symbols.begin(), symbols.end(), [&](const SymbolInfo& symbol) { return symbol.id == id; });
    return it == symbols.end() ? nullptr : &(*it);
}

}  // namespace oikumene
