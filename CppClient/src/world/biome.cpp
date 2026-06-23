#include "oikumene/world/biome.hpp"

namespace oikumene {

std::string ToString(Biome biome) {
    switch (biome) {
        case Biome::Ocean:
            return "Ocean";
        case Biome::Coast:
            return "Coast";
        case Biome::Lake:
            return "Lake";
        case Biome::River:
            return "River";
        case Biome::Desert:
            return "Desert";
        case Biome::Grassland:
            return "Grassland";
        case Biome::Forest:
            return "Forest";
        case Biome::Rainforest:
            return "Rainforest";
        case Biome::Wetland:
            return "Wetland";
        case Biome::Tundra:
            return "Tundra";
        case Biome::Snow:
            return "Snow";
        case Biome::Hill:
            return "Hill";
        case Biome::Mountain:
            return "Mountain";
    }
    return "Unknown";
}

}  // namespace oikumene
