#pragma once

#include <cstdint>
#include <string>

namespace oikumene {

enum class ResourceKind : std::uint8_t {
    None,
    Wood,
    Bamboo,
    Horse,
    Copper,
    Tin,
    ShallowIron,
    ShallowCoal,
    MeteoricIron,
    Gold,
    Silver,
    Stone,
    Clay,
    Salt,
};

[[nodiscard]] std::string ToString(ResourceKind resource);

}  // namespace oikumene
