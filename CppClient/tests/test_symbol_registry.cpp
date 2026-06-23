#include <cassert>
#include <iostream>
#include <set>
#include <string>

#include "oikumene/render/symbol_registry.hpp"

namespace {

int Count(oikumene::SymbolCategory category) {
    int count = 0;
    for (const auto& symbol : oikumene::LegendSymbols()) {
        count += symbol.category == category && symbol.visible_in_legend ? 1 : 0;
    }
    return count;
}

void TestRequiredCategoriesHaveSymbols() {
    using namespace oikumene;
    assert(Count(SymbolCategory::Biome) >= 10);
    assert(Count(SymbolCategory::Water) >= 3);
    assert(Count(SymbolCategory::Resource) >= 10);
    assert(Count(SymbolCategory::Improvement) >= 5);
    assert(Count(SymbolCategory::Entity) >= 4);
    assert(Count(SymbolCategory::Polity) >= 3);
    assert(Count(SymbolCategory::Overlay) >= 2);
}

void TestRequiredSymbolsExist() {
    using namespace oikumene;
    const char* ids[] = {
        "biome.ocean",           "biome.grassland",       "biome.forest",
        "biome.mountain",        "water.river",           "resource.horse",
        "resource.copper",       "resource.iron",         "resource.gold",
        "improvement.farm",      "improvement.lumbercamp", "improvement.pasture",
        "improvement.shallowmine", "entity.band",          "entity.village",
        "entity.capital",        "polity.controlled",     "polity.contested",
        "overlay.selection",
    };
    for (const char* id : ids) {
        assert(FindSymbolById(id) != nullptr);
    }
}

void TestNoDuplicateIdsAndNoEmptyText() {
    std::set<std::string> ids;
    for (const auto& symbol : oikumene::LegendSymbols()) {
        assert(!symbol.id.empty());
        assert(!symbol.label.empty());
        assert(!symbol.description.empty());
        assert(!symbol.shape.empty());
        assert(ids.insert(symbol.id).second);
    }
}

}  // namespace

int main() {
    TestRequiredCategoriesHaveSymbols();
    TestRequiredSymbolsExist();
    TestNoDuplicateIdsAndNoEmptyText();

    std::cout << "oikumene_symbol_registry_tests passed\n";
    return 0;
}
