#include <cassert>
#include <iostream>

#include "oikumene/sim/control_field.hpp"

namespace {

oikumene::World MakePlainWorld(int width = 12, int height = 7) {
    using namespace oikumene;
    World world(width, height, 11);
    for (auto& tile : world.Tiles()) {
        tile.is_ocean = false;
        tile.is_lake = false;
        tile.biome = Biome::Grassland;
        tile.movement_cost = 1.0F;
        tile.fertility = 0.5F;
    }
    return world;
}

oikumene::Settlement MakeVillage(int id, int x, int y, int population = 160) {
    oikumene::Settlement settlement;
    settlement.id = id;
    settlement.x = x;
    settlement.y = y;
    settlement.level = oikumene::SettlementLevel::Village;
    settlement.population = population;
    settlement.stockpile.food = 120.0F;
    return settlement;
}

oikumene::Polity MakePolity(oikumene::PolityId id, int capital_id) {
    oikumene::Polity polity;
    polity.id = id;
    polity.name = "Test " + std::to_string(id);
    polity.capital_settlement_id = capital_id;
    polity.member_settlement_ids.push_back(capital_id);
    polity.admin_range = 28.0F;
    return polity;
}

void TestMountainBarrierReducesControlSpread() {
    using namespace oikumene;
    World plain = MakePlainWorld();
    World mountain = MakePlainWorld();
    for (int y = 0; y < mountain.Height(); ++y) {
        auto& tile = mountain.At(5, y);
        tile.biome = Biome::Mountain;
        tile.movement_cost = 5.2F;
    }

    std::vector<Settlement> settlements{MakeVillage(0, 1, 3)};
    std::vector<Polity> plain_polities{MakePolity(0, 0)};
    std::vector<Polity> mountain_polities{MakePolity(0, 0)};

    const auto plain_stats = RecomputeControlField(plain, settlements, plain_polities);
    const auto mountain_stats = RecomputeControlField(mountain, settlements, mountain_polities);
    assert(plain_stats.controlled_land_tiles > 0);
    assert(mountain_stats.controlled_land_tiles > 0);
    assert(plain.At(10, 3).control_strength > mountain.At(10, 3).control_strength);
}

void TestRiverCorridorImprovesControlSpread() {
    using namespace oikumene;
    World dry = MakePlainWorld();
    World river = MakePlainWorld();
    for (int x = 0; x < river.Width(); ++x) {
        river.At(x, 3).has_river = true;
        river.At(x, 3).biome = Biome::River;
        river.At(x, 3).movement_cost = 0.8F;
    }
    for (auto& tile : dry.Tiles()) {
        tile.biome = Biome::Hill;
        tile.movement_cost = 2.1F;
    }

    std::vector<Settlement> settlements{MakeVillage(0, 1, 3)};
    std::vector<Polity> dry_polities{MakePolity(0, 0)};
    std::vector<Polity> river_polities{MakePolity(0, 0)};

    const auto dry_stats = RecomputeControlField(dry, settlements, dry_polities);
    const auto river_stats = RecomputeControlField(river, settlements, river_polities);
    assert(dry_stats.controlled_land_tiles > 0);
    assert(river_stats.controlled_land_tiles > 0);
    assert(river.At(10, 3).control_strength > dry.At(10, 3).control_strength);
}

void TestOceanBlocksEarlyControl() {
    using namespace oikumene;
    World world = MakePlainWorld();
    for (int y = 0; y < world.Height(); ++y) {
        auto& tile = world.At(5, y);
        tile.is_ocean = true;
        tile.biome = Biome::Ocean;
    }
    std::vector<Settlement> settlements{MakeVillage(0, 1, 3)};
    std::vector<Polity> polities{MakePolity(0, 0)};
    const auto stats = RecomputeControlField(world, settlements, polities);
    assert(stats.controlled_land_tiles > 0);

    for (int x = 6; x < world.Width(); ++x) {
        for (int y = 0; y < world.Height(); ++y) {
            assert(world.At(x, y).controller_polity_id == kInvalidPolityId);
        }
    }
}

void TestContestedTilesAppearBetweenClosePolities() {
    using namespace oikumene;
    World world = MakePlainWorld(16, 7);
    std::vector<Settlement> settlements{MakeVillage(0, 4, 3), MakeVillage(1, 11, 3)};
    std::vector<Polity> polities{MakePolity(0, 0), MakePolity(1, 1)};

    const auto stats = RecomputeControlField(world, settlements, polities);
    assert(stats.contested_tiles > 0);
}

}  // namespace

int main() {
    TestMountainBarrierReducesControlSpread();
    TestRiverCorridorImprovesControlSpread();
    TestOceanBlocksEarlyControl();
    TestContestedTilesAppearBetweenClosePolities();

    std::cout << "oikumene_control_field_tests passed\n";
    return 0;
}
