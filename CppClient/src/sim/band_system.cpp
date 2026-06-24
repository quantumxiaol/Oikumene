#include "oikumene/sim/band_system.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace oikumene {
namespace {

int DistanceSquared(int ax, int ay, int bx, int by) {
    const int dx = ax - bx;
    const int dy = ay - by;
    return dx * dx + dy * dy;
}

float CandidateScore(const World& world, const Tile& tile, const std::vector<Settlement>& settlements, int from_x,
                     int from_y) {
    if (!IsValidSettlementTile(tile)) {
        return -1000.0F;
    }

    const int distance = std::abs(tile.x - from_x) + std::abs(tile.y - from_y);
    float score = tile.settlement_score + ForageYield(tile) * 0.025F - tile.movement_cost * 0.025F -
                  static_cast<float>(distance) * 0.010F;
    if (IsSettlementNearby(settlements, tile.x, tile.y, 6)) {
        score -= 0.65F;
    }
    if (tile.is_coast || tile.has_river) {
        score += 0.05F;
    }
    (void)world;
    return score;
}

std::pair<int, int> StepToward(const World& world, const Band& band, int target_x, int target_y) {
    int best_x = band.x;
    int best_y = band.y;
    float best_score = static_cast<float>(DistanceSquared(band.x, band.y, target_x, target_y));

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            const int nx = band.x + dx;
            const int ny = band.y + dy;
            if (!world.InBounds(nx, ny) || !IsValidBandTile(world.At(nx, ny))) {
                continue;
            }
            const auto& tile = world.At(nx, ny);
            const float score =
                static_cast<float>(DistanceSquared(nx, ny, target_x, target_y)) + tile.movement_cost * 0.25F;
            if (score < best_score) {
                best_score = score;
                best_x = nx;
                best_y = ny;
            }
        }
    }

    return {best_x, best_y};
}

std::string TileReason(const Tile& tile) {
    std::ostringstream stream;
    stream << "score " << tile.settlement_score << ", fertility " << tile.fertility << ", river "
           << (tile.has_river ? "yes" : "no") << ", coast " << (tile.is_coast ? "yes" : "no") << ", biome "
           << ToString(tile.biome);
    return stream.str();
}

bool CanSettle(const World& world, const SimulationParams& params, const Band& band,
               const std::vector<Settlement>& settlements) {
    if (!band.active || band.population < 20 || band.food < 10.0F || !world.InBounds(band.x, band.y)) {
        return false;
    }
    const auto& tile = world.At(band.x, band.y);
    return IsValidSettlementTile(tile) && tile.settlement_score >= params.settlement_threshold &&
           !IsSettlementNearby(settlements, band.x, band.y, params.settlement_exclusion_radius);
}

void FoundSettlement(const World& world, Turn turn, Band& band, std::vector<Settlement>& settlements,
                     EventLog& event_log) {
    const auto& tile = world.At(band.x, band.y);
    const int settlement_id = static_cast<int>(settlements.size());
    Settlement settlement;
    settlement.id = settlement_id;
    settlement.x = band.x;
    settlement.y = band.y;
    settlement.founder_band_id = band.id;
    settlement.population = band.population;
    settlement.stockpile.food = std::max(15.0F, band.food);
    settlement.stockpile.wood =
        tile.resource == ResourceKind::Wood || tile.resource == ResourceKind::Bamboo ? 12.0F : 6.0F;
    settlements.push_back(settlement);

    band.active = false;
    band.state = BandState::Settled;
    band.current_tile_score = tile.settlement_score;
    band.best_seen_score = tile.settlement_score;

    std::ostringstream summary;
    summary << "Band " << band.id << " founded Camp " << settlement_id << " at " << band.x << "," << band.y << ": "
            << TileReason(tile);
    band.last_decision_reason = summary.str();
    event_log.Add(SimEvent{
        .turn = turn,
        .type = EventType::SettlementFounded,
        .actor_id = settlement_id,
        .x = band.x,
        .y = band.y,
        .summary = summary.str(),
    });
}

} // namespace

bool IsValidBandTile(const Tile& tile) {
    return !tile.is_ocean && !tile.is_lake && tile.biome != Biome::Mountain && tile.biome != Biome::Snow;
}

bool IsValidSettlementTile(const Tile& tile) {
    return IsValidBandTile(tile) && tile.biome != Biome::Tundra && tile.biome != Biome::Wetland;
}

float ForageYield(const Tile& tile) {
    if (!IsValidBandTile(tile)) {
        return 0.0F;
    }

    float yield = tile.fertility * 8.0F;
    if (tile.biome == Biome::Forest || tile.biome == Biome::Rainforest) {
        yield += 3.0F;
    }
    if (tile.biome == Biome::Grassland || tile.biome == Biome::Coast) {
        yield += 2.0F;
    }
    if (tile.has_river || tile.is_coast || tile.is_lake) {
        yield += 3.0F;
    }
    if (tile.resource == ResourceKind::Horse) {
        yield += 0.8F;
    }
    if (tile.biome == Biome::Desert || tile.biome == Biome::Tundra || tile.biome == Biome::Snow ||
        tile.biome == Biome::Mountain) {
        yield *= 0.25F;
    }
    return std::max(0.0F, yield);
}

bool IsSettlementNearby(const std::vector<Settlement>& settlements, int x, int y, int radius) {
    const int radius_squared = radius * radius;
    return std::any_of(settlements.begin(), settlements.end(), [&](const Settlement& settlement) {
        return DistanceSquared(settlement.x, settlement.y, x, y) <= radius_squared;
    });
}

void BandSystem::InitializeBands(const World& world, const SimulationParams&, int count, std::vector<Band>& bands) {
    std::vector<const Tile*> candidates;
    candidates.reserve(world.Tiles().size());
    for (const auto& tile : world.Tiles()) {
        if (IsValidBandTile(tile)) {
            candidates.push_back(&tile);
        }
    }

    std::sort(candidates.begin(), candidates.end(),
              [](const Tile* lhs, const Tile* rhs) { return lhs->settlement_score > rhs->settlement_score; });

    bands.clear();
    const int stride = std::max(1, static_cast<int>(candidates.size()) / std::max(1, count * 3));
    for (int id = 0; id < count && !candidates.empty(); ++id) {
        const auto* tile = candidates[static_cast<std::size_t>((id * stride * 2 + id * 7) % candidates.size())];
        Band band;
        band.id = id;
        band.x = tile->x;
        band.y = tile->y;
        band.population = 24 + static_cast<int>((world.Seed() + static_cast<std::uint64_t>(id) * 13ULL) % 27ULL);
        band.food = 18.0F + static_cast<float>((world.Seed() + static_cast<std::uint64_t>(id) * 5ULL) % 16ULL);
        band.current_tile_score = tile->settlement_score;
        band.best_seen_score = tile->settlement_score;
        band.forage_yield_last_turn = ForageYield(*tile);
        std::ostringstream reason;
        reason << "Band " << band.id << " spawned at " << band.x << "," << band.y << ": " << TileReason(*tile);
        band.last_decision_reason = reason.str();
        bands.push_back(band);
    }
}

void BandSystem::UpdateBands(const World& world, const SimulationParams& params, Turn turn, std::vector<Band>& bands,
                             std::vector<Settlement>& settlements, EventLog& event_log) {
    for (auto& band : bands) {
        if (!band.active) {
            continue;
        }

        ++band.turns_alive;
        const auto& current_tile = world.At(band.x, band.y);
        band.current_tile_score = current_tile.settlement_score;
        band.forage_yield_last_turn = ForageYield(current_tile);
        band.food -= static_cast<float>(band.population) * params.band_food_consumption_per_person;
        band.food += band.forage_yield_last_turn * 0.42F;

        if (CanSettle(world, params, band, settlements)) {
            band.state = BandState::Settling;
            FoundSettlement(world, turn, band, settlements, event_log);
            continue;
        }

        const float current_score = CandidateScore(world, current_tile, settlements, band.x, band.y);
        float best_score = current_score;
        float best_tile_score = current_tile.settlement_score;
        int best_x = band.x;
        int best_y = band.y;

        for (int dy = -params.band_scan_radius; dy <= params.band_scan_radius; ++dy) {
            for (int dx = -params.band_scan_radius; dx <= params.band_scan_radius; ++dx) {
                const int x = band.x + dx;
                const int y = band.y + dy;
                if (!world.InBounds(x, y)) {
                    continue;
                }
                const auto& tile = world.At(x, y);
                const float score = CandidateScore(world, tile, settlements, band.x, band.y);
                if (score > best_score + 0.045F) {
                    best_score = score;
                    best_tile_score = tile.settlement_score;
                    best_x = x;
                    best_y = y;
                }
            }
        }
        band.best_seen_score = best_tile_score;

        if (best_x != band.x || best_y != band.y) {
            const auto [next_x, next_y] = StepToward(world, band, best_x, best_y);
            if (next_x != band.x || next_y != band.y) {
                band.state = BandState::Migrating;
                band.target_x = best_x;
                band.target_y = best_y;
                band.x = next_x;
                band.y = next_y;
                const auto& target_tile = world.At(best_x, best_y);
                std::ostringstream reason;
                reason << "Band " << band.id << " migrated toward better site: score " << current_tile.settlement_score
                       << " -> " << target_tile.settlement_score << ", fertility " << target_tile.fertility
                       << ", river " << (target_tile.has_river ? "yes" : "no") << ", forage "
                       << band.forage_yield_last_turn;
                band.last_decision_reason = reason.str();
                event_log.Add(SimEvent{
                    .turn = turn,
                    .type = EventType::BandMigrated,
                    .actor_id = band.id,
                    .x = band.x,
                    .y = band.y,
                    .summary = band.last_decision_reason,
                });
                continue;
            }
        }

        band.state =
            band.food < static_cast<float>(band.population) * 0.35F ? BandState::Foraging : BandState::Exploring;
        std::ostringstream reason;
        reason << "Band " << band.id << " " << (band.state == BandState::Foraging ? "foraged" : "explored")
               << ": current score " << current_tile.settlement_score << ", best nearby " << band.best_seen_score
               << ", forage " << band.forage_yield_last_turn << ", food " << band.food;
        band.last_decision_reason = reason.str();
    }
}

} // namespace oikumene
