#pragma once

#include <cstdint>

#include <raylib.h>

#include "oikumene/app/app_state.hpp"
#include "oikumene/render/selection.hpp"

namespace oikumene {

void BuildSimulation(AppState& state, std::uint64_t seed);
void StepTurns(AppState& state, int turns);
void ApplySelection(AppState& state, Selection selection);
void HandleInput(AppState& state);
void AdvanceAutoRun(AppState& state);
void HandleFocusPause(AppState& state);
void UpdateHover(AppState& state);

} // namespace oikumene
