#include "oikumene/app/command_line.hpp"

#include <sstream>

namespace oikumene {
namespace {

bool ParseWindowSize(const std::string& value, int& width, int& height) {
    const auto separator = value.find('x');
    if (separator == std::string::npos) {
        return false;
    }

    try {
        width = std::stoi(value.substr(0, separator));
        height = std::stoi(value.substr(separator + 1));
        return width > 0 && height > 0;
    } catch (const std::exception&) {
        return false;
    }
}

bool NeedValue(const std::vector<std::string>& args, std::size_t index, std::string& error) {
    if (index + 1 < args.size()) {
        return true;
    }
    error = "missing value after " + args[index];
    return false;
}

}  // namespace

CommandLineResult ParseCommandLine(const std::vector<std::string>& args, AppConfig base) {
    CommandLineResult result;
    result.config = base;

    for (std::size_t i = 0; i < args.size(); ++i) {
        const auto& arg = args[i];
        try {
            if (arg == "--help" || arg == "-h") {
                result.show_help = true;
                return result;
            }
            if (arg == "--seed") {
                if (!NeedValue(args, i, result.error)) {
                    return result;
                }
                result.config.simulation.default_seed = static_cast<std::uint64_t>(std::stoull(args[++i]));
                continue;
            }
            if (arg == "--width") {
                if (!NeedValue(args, i, result.error)) {
                    return result;
                }
                result.config.simulation.world_width = std::stoi(args[++i]);
                continue;
            }
            if (arg == "--height") {
                if (!NeedValue(args, i, result.error)) {
                    return result;
                }
                result.config.simulation.world_height = std::stoi(args[++i]);
                continue;
            }
            if (arg == "--bands") {
                if (!NeedValue(args, i, result.error)) {
                    return result;
                }
                result.config.simulation.initial_bands = std::stoi(args[++i]);
                continue;
            }
            if (arg == "--window") {
                if (!NeedValue(args, i, result.error)) {
                    return result;
                }
                if (!ParseWindowSize(args[++i], result.config.window.width, result.config.window.height)) {
                    result.error = "invalid --window value; expected WIDTHxHEIGHT";
                    return result;
                }
                continue;
            }
            if (arg == "--turns-per-second") {
                if (!NeedValue(args, i, result.error)) {
                    return result;
                }
                result.config.simulation.turns_per_second = std::stof(args[++i]);
                continue;
            }
            if (arg == "--fullscreen") {
                result.config.window.fullscreen = true;
                continue;
            }
            if (arg == "--auto-run") {
                result.config.simulation.auto_run = true;
                continue;
            }

            result.error = "unknown argument: " + arg;
            return result;
        } catch (const std::exception& error) {
            result.error = "invalid value for " + arg + ": " + error.what();
            return result;
        }
    }

    if (result.config.simulation.world_width <= 0) {
        result.error = "--width must be positive";
        return result;
    }
    if (result.config.simulation.world_height <= 0) {
        result.error = "--height must be positive";
        return result;
    }
    if (result.config.simulation.initial_bands <= 0) {
        result.error = "--bands must be positive";
        return result;
    }
    if (result.config.simulation.turns_per_second <= 0.0F) {
        result.error = "--turns-per-second must be positive";
        return result;
    }

    return result;
}

std::string CommandLineUsage() {
    std::ostringstream stream;
    stream << "usage: oikumene_app [--seed N] [--width N] [--height N] [--bands N] [--window WIDTHxHEIGHT]\n"
           << "                    [--turns-per-second N] [--fullscreen] [--auto-run]\n";
    return stream.str();
}

}  // namespace oikumene
