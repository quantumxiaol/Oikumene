#pragma once

#include "oikumene/app/app_config.hpp"

namespace oikumene {

class OikumeneApp {
  public:
    explicit OikumeneApp(AppConfig config);

    int Run();

  private:
    AppConfig config_;
};

} // namespace oikumene
