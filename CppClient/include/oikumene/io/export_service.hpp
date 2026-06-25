#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "oikumene/app/app_state.hpp"
#include "oikumene/render/map_layer.hpp"
#include "oikumene/world/world_generation_report.hpp"

namespace oikumene {

[[nodiscard]] std::filesystem::path RunsDirectory();
[[nodiscard]] std::filesystem::path WorldgenDirectory(std::uint64_t seed);
[[nodiscard]] std::string LayerFilename(MapLayer layer);
[[nodiscard]] bool WriteReportJson(const WorldGenerationReport& report, std::filesystem::path path);
void CapturePendingScreenshot(AppState& state);

} // namespace oikumene
