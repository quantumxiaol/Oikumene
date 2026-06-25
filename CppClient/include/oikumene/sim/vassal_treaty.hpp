#pragma once

#include <string>

#include "oikumene/core/types.hpp"

namespace oikumene {

enum class VassalTreatyStatus {
    Active,
    Broken,
};

[[nodiscard]] std::string ToString(VassalTreatyStatus status);

struct VassalTreaty {
    int id = -1;
    int source_occupation_id = -1;
    PolityId overlord_polity_id = kInvalidPolityId;
    PolityId subject_polity_id = kInvalidPolityId;

    VassalTreatyStatus status = VassalTreatyStatus::Active;
    Turn started_turn = 0;
    Turn last_update_turn = 0;
    Turn ended_turn = -1;

    float strength = 0.0F;
    float autonomy = 0.0F;
    float tribute_rate = 0.0F;
    float protection = 0.0F;
    float loyalty = 0.0F;
    float liberty_desire = 0.0F;
    float tribute_due = 0.0F;
    float military_obligation = 0.0F;

    std::string reason;
};

} // namespace oikumene
