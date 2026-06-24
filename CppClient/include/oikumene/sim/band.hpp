#pragma once

#include <string>

namespace oikumene {

enum class BandState {
    Exploring,
    Foraging,
    Migrating,
    Settling,
    Settled,
};

[[nodiscard]] std::string ToString(BandState state);

struct Band {
    int id = -1;
    int x = 0;
    int y = 0;
    int population = 30;
    float food = 20.0F;
    int target_x = -1;
    int target_y = -1;
    int turns_alive = 0;
    BandState state = BandState::Exploring;
    bool active = true;
    std::string last_decision_reason;
    float current_tile_score = 0.0F;
    float best_seen_score = 0.0F;
    float forage_yield_last_turn = 0.0F;
};

} // namespace oikumene
