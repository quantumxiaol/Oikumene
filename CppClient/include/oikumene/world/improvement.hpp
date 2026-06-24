#pragma once

#include <cstdint>
#include <string>

namespace oikumene {

enum class ImprovementKind : std::uint8_t {
    None,
    ForagingGround,
    Farm,
    LumberCamp,
    Pasture,
    ShallowMine,
    Road,
};

[[nodiscard]] std::string ToString(ImprovementKind improvement);

} // namespace oikumene
