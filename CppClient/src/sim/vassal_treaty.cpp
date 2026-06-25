#include "oikumene/sim/vassal_treaty.hpp"

namespace oikumene {

std::string ToString(VassalTreatyStatus status) {
    switch (status) {
    case VassalTreatyStatus::Active:
        return "Active";
    case VassalTreatyStatus::Broken:
        return "Broken";
    }
    return "Unknown";
}

} // namespace oikumene
