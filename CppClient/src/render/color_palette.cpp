#include "oikumene/render/color_palette.hpp"

#include <algorithm>
#include <cmath>

namespace oikumene {
namespace {

[[nodiscard]] unsigned char Channel(float value) {
    return static_cast<unsigned char>(std::clamp(value, 0.0F, 255.0F));
}

[[nodiscard]] Color LerpColor(Color a, Color b, float t) {
    t = std::clamp(t, 0.0F, 1.0F);
    return Color{
        Channel(static_cast<float>(a.r) + (static_cast<float>(b.r) - static_cast<float>(a.r)) * t),
        Channel(static_cast<float>(a.g) + (static_cast<float>(b.g) - static_cast<float>(a.g)) * t),
        Channel(static_cast<float>(a.b) + (static_cast<float>(b.b) - static_cast<float>(a.b)) * t),
        255,
    };
}

[[nodiscard]] Color Heat(float value) {
    return LerpColor(Color{42, 89, 190, 255}, Color{230, 74, 46, 255}, value);
}

[[nodiscard]] Color Moisture(float value) {
    return LerpColor(Color{164, 128, 68, 255}, Color{44, 126, 210, 255}, value);
}

[[nodiscard]] Color Greens(float value) {
    return LerpColor(Color{70, 83, 58, 255}, Color{61, 174, 90, 255}, value);
}

} // namespace

Color ColorForBiome(Biome biome) {
    switch (biome) {
    case Biome::Ocean:
        return Color{35, 73, 137, 255};
    case Biome::Coast:
        return Color{73, 150, 184, 255};
    case Biome::Lake:
        return Color{53, 112, 184, 255};
    case Biome::River:
        return Color{45, 132, 210, 255};
    case Biome::Desert:
        return Color{196, 164, 93, 255};
    case Biome::Grassland:
        return Color{122, 166, 84, 255};
    case Biome::Forest:
        return Color{55, 118, 70, 255};
    case Biome::Rainforest:
        return Color{35, 103, 72, 255};
    case Biome::Wetland:
        return Color{65, 116, 103, 255};
    case Biome::Tundra:
        return Color{138, 151, 139, 255};
    case Biome::Snow:
        return Color{226, 230, 222, 255};
    case Biome::Hill:
        return Color{124, 113, 77, 255};
    case Biome::Mountain:
        return Color{112, 104, 96, 255};
    }
    return MAGENTA;
}

Color ColorForResource(ResourceKind resource) {
    switch (resource) {
    case ResourceKind::None:
        return Color{48, 52, 55, 255};
    case ResourceKind::Wood:
        return Color{40, 122, 68, 255};
    case ResourceKind::Bamboo:
        return Color{83, 184, 88, 255};
    case ResourceKind::Horse:
        return Color{151, 98, 52, 255};
    case ResourceKind::Copper:
        return Color{205, 112, 52, 255};
    case ResourceKind::Tin:
        return Color{178, 188, 190, 255};
    case ResourceKind::ShallowIron:
        return Color{91, 83, 77, 255};
    case ResourceKind::ShallowCoal:
        return Color{22, 24, 26, 255};
    case ResourceKind::MeteoricIron:
        return Color{154, 177, 202, 255};
    case ResourceKind::Gold:
        return Color{224, 181, 54, 255};
    case ResourceKind::Silver:
        return Color{190, 196, 206, 255};
    case ResourceKind::Stone:
        return Color{136, 130, 121, 255};
    case ResourceKind::Clay:
        return Color{176, 91, 60, 255};
    case ResourceKind::Salt:
        return Color{232, 226, 202, 255};
    }
    return MAGENTA;
}

Color ColorForPolity(PolityId polity_id) {
    static constexpr Color palette[] = {
        Color{225, 92, 86, 255},   Color{86, 158, 226, 255}, Color{98, 188, 116, 255}, Color{218, 172, 72, 255},
        Color{176, 112, 210, 255}, Color{82, 190, 184, 255}, Color{228, 126, 67, 255}, Color{168, 184, 72, 255},
    };
    if (polity_id == kInvalidPolityId) {
        return Color{42, 46, 50, 255};
    }
    constexpr int palette_size = static_cast<int>(sizeof(palette) / sizeof(palette[0]));
    const auto index = static_cast<std::size_t>(std::abs(polity_id) % palette_size);
    return palette[index];
}

Color ColorForTile(const Tile& tile, MapLayer layer) {
    switch (layer) {
    case MapLayer::Biome:
        return ColorForBiome(tile.biome);
    case MapLayer::Elevation:
        return LerpColor(Color{36, 68, 128, 255}, Color{232, 232, 226, 255}, tile.elevation);
    case MapLayer::Rainfall:
        return Moisture(tile.rainfall);
    case MapLayer::Temperature:
        return Heat(tile.temperature);
    case MapLayer::Fertility:
        return Greens(tile.fertility);
    case MapLayer::Resources:
        return LerpColor(ColorForBiome(tile.biome), Color{20, 22, 24, 255}, 0.50F);
    case MapLayer::SettlementScore:
        return LerpColor(Color{52, 50, 56, 255}, Color{238, 214, 93, 255}, tile.settlement_score);
    case MapLayer::PolityControl:
        return LerpColor(ColorForBiome(tile.biome), Color{18, 22, 26, 255}, 0.48F);
    case MapLayer::RouteNetwork:
        return LerpColor(ColorForBiome(tile.biome), Color{18, 22, 26, 255}, 0.62F);
    }
    return MAGENTA;
}

} // namespace oikumene
