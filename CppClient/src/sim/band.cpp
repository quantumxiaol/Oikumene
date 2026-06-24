#include "oikumene/sim/band.hpp"

namespace oikumene {

std::string ToString(BandState state) {
    switch (state) {
    case BandState::Exploring:
        return "Exploring";
    case BandState::Foraging:
        return "Foraging";
    case BandState::Migrating:
        return "Migrating";
    case BandState::Settling:
        return "Settling";
    case BandState::Settled:
        return "Settled";
    }
    return "Unknown";
}

} // namespace oikumene
