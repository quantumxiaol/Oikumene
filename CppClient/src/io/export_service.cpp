#include "oikumene/io/export_service.hpp"

#include <cctype>
#include <fstream>

#include <raylib.h>

namespace oikumene {

std::filesystem::path RunsDirectory() {
    const auto cwd = std::filesystem::current_path();
    if (cwd.filename() == "CppClient") {
        return cwd.parent_path() / "runs";
    }
    if (std::filesystem::exists(cwd / "CppClient")) {
        return cwd / "runs";
    }
    return cwd / "runs";
}

std::filesystem::path WorldgenDirectory(std::uint64_t seed) {
    return RunsDirectory() / ("worldgen_seed_" + std::to_string(seed));
}

std::string LayerFilename(MapLayer layer) {
    std::string name = ToString(layer);
    for (auto& ch : name) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return "layer_" + name + ".png";
}

bool WriteReportJson(const WorldGenerationReport& report, std::filesystem::path path) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path);
    if (!output) {
        return false;
    }
    output << ToJson(report).dump(2) << '\n';
    return true;
}

void CapturePendingScreenshot(AppState& state) {
    if (!state.pending_screenshot.has_value()) {
        return;
    }

    TakeScreenshot(state.pending_screenshot->string().c_str());
    state.status_message = "Exported screenshot: " + state.pending_screenshot->string();
    state.pending_screenshot.reset();
}

} // namespace oikumene
