#include "oikumene/render/map_layer.hpp"

#include <raylib.h>

namespace oikumene {

std::string ToString(MapLayer layer) {
    switch (layer) {
        case MapLayer::Biome:
            return "Biome";
        case MapLayer::Elevation:
            return "Elevation";
        case MapLayer::Rainfall:
            return "Rainfall";
        case MapLayer::Temperature:
            return "Temperature";
        case MapLayer::Fertility:
            return "Fertility";
        case MapLayer::Resources:
            return "Resources";
        case MapLayer::SettlementScore:
            return "SettlementScore";
    }
    return "Unknown";
}

MapLayer MapLayerFromHotkey(int key) {
    switch (key) {
        case KEY_ONE:
            return MapLayer::Biome;
        case KEY_TWO:
            return MapLayer::Elevation;
        case KEY_THREE:
            return MapLayer::Rainfall;
        case KEY_FOUR:
            return MapLayer::Temperature;
        case KEY_FIVE:
            return MapLayer::Fertility;
        case KEY_SIX:
            return MapLayer::Resources;
        case KEY_SEVEN:
            return MapLayer::SettlementScore;
        default:
            return MapLayer::Biome;
    }
}

}  // namespace oikumene
