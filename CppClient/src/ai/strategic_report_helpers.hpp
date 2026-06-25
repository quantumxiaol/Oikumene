#pragma once

#include <vector>

#include "oikumene/core/simulation.hpp"

namespace oikumene::strategic_report {

[[nodiscard]] float Clamp01(float value);
[[nodiscard]] const Polity* PolityById(const std::vector<Polity>& polities, PolityId id);
[[nodiscard]] const Settlement* SettlementById(const std::vector<Settlement>& settlements, int id);
[[nodiscard]] bool InvolvesPolity(const DiplomacyRelation& relation, PolityId id);
[[nodiscard]] PolityId OtherPolity(const DiplomacyRelation& relation, PolityId id);
[[nodiscard]] float DirectedValue(float a_to_b, float b_to_a, const DiplomacyRelation& relation, PolityId from_id);
[[nodiscard]] float MaxRelationRisk(const DiplomacyRelation& relation);
[[nodiscard]] float RelationSortScore(const DiplomacyRelation& relation, PolityId polity_id);

} // namespace oikumene::strategic_report
