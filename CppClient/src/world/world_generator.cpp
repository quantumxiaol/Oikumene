#include "oikumene/world/world_generator.hpp"

namespace oikumene {

World WorldGenerator::Generate(const WorldGenerationParams& params) {
    return WorldGenerator(params).Run();
}

WorldGenerator::WorldGenerator(const WorldGenerationParams& params)
    : params_(params), world_(params.width, params.height, params.seed), rng_(params.seed) {}

World WorldGenerator::Run() {
    GenerateElevation();
    ClassifyOceanAndLand();
    GenerateTemperature();
    GenerateRainfall();
    GenerateRiversAndLakes();
    GenerateBiomes();
    GenerateResources();
    ComputeMovementCost();
    ComputeSettlementScore();
    return world_;
}

} // namespace oikumene
