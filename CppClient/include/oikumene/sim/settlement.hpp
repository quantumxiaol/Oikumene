#pragma once

#include <string>

#include "oikumene/sim/stockpile.hpp"

namespace oikumene {

enum class SettlementLevel {
    Camp,
    Village,
};

[[nodiscard]] std::string ToString(SettlementLevel level);

struct Settlement {
    int id = -1;
    int x = 0;
    int y = 0;
    int founder_band_id = -1;
    int population = 30;
    SettlementLevel level = SettlementLevel::Camp;
    Stockpile stockpile;
    int turns_since_founded = 0;
};

}  // namespace oikumene
