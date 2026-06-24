#include "oikumene/sim/settlement.hpp"

namespace oikumene {

std::string ToString(SettlementLevel level) {
    switch (level) {
    case SettlementLevel::Camp:
        return "Camp";
    case SettlementLevel::Village:
        return "Village";
    }
    return "Unknown";
}

} // namespace oikumene
