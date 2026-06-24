#pragma once

#include <string>

namespace oikumene {

enum class MapLayer {
    Biome,
    Elevation,
    Rainfall,
    Temperature,
    Fertility,
    Resources,
    SettlementScore,
    PolityControl,
    RouteNetwork,
    TradeNetwork,
};

[[nodiscard]] std::string ToString(MapLayer layer);
[[nodiscard]] MapLayer MapLayerFromHotkey(int key);

} // namespace oikumene
