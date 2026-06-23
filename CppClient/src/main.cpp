#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "oikumene/app/app_config.hpp"
#include "oikumene/app/command_line.hpp"
#include "oikumene/app/oikumene_app.hpp"

int main(int argc, char** argv) {
    const auto settings_path = std::filesystem::path("config") / "settings.json";
    auto config = oikumene::LoadAppConfig(settings_path);

    std::vector<std::string> args;
    args.reserve(static_cast<std::size_t>(argc > 1 ? argc - 1 : 0));
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    const auto parsed = oikumene::ParseCommandLine(args, config);
    if (!parsed.error.empty()) {
        std::cerr << parsed.error << "\n" << oikumene::CommandLineUsage();
        return 1;
    }
    if (parsed.show_help) {
        std::cout << oikumene::CommandLineUsage();
        return 0;
    }

    oikumene::OikumeneApp app(parsed.config);
    return app.Run();
}
