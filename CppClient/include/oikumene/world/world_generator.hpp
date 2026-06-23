#pragma once

#include <cstdint>
#include <random>

#include "oikumene/world/world.hpp"
#include "oikumene/world/world_generation_params.hpp"

namespace oikumene {

class WorldGenerator {
public:
    [[nodiscard]] static World Generate(const WorldGenerationParams& params);

private:
    explicit WorldGenerator(const WorldGenerationParams& params);

    [[nodiscard]] World Run();

    void GenerateElevation();
    void ClassifyOceanAndLand();
    void GenerateTemperature();
    void GenerateRainfall();
    void GenerateRiversAndLakes();
    void GenerateBiomes();
    void GenerateResources();
    void ComputeMovementCost();
    void ComputeSettlementScore();

    [[nodiscard]] float Noise2D(float x, float y, int salt, float frequency) const;
    [[nodiscard]] float Random01(int x, int y, int salt) const;
    [[nodiscard]] bool IsAdjacentToWater(int x, int y) const;
    [[nodiscard]] bool HasNeighborBiome(int x, int y, Biome biome) const;

    WorldGenerationParams params_;
    World world_;
    std::mt19937_64 rng_;
};

}  // namespace oikumene
