#pragma once

#include <string>
#include <vector>

#include "oikumene/app/app_config.hpp"

namespace oikumene {

struct CommandLineResult {
    AppConfig config;
    bool show_help = false;
    std::string error;
};

[[nodiscard]] CommandLineResult ParseCommandLine(const std::vector<std::string>& args, AppConfig base = {});
[[nodiscard]] std::string CommandLineUsage();

} // namespace oikumene
