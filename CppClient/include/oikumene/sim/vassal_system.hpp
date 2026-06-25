#pragma once

#include <vector>

#include "oikumene/core/types.hpp"
#include "oikumene/sim/diplomacy.hpp"
#include "oikumene/sim/polity.hpp"
#include "oikumene/sim/vassal_treaty.hpp"

namespace oikumene {

class VassalSystem {
  public:
    static void Reset(std::vector<VassalTreaty>& treaties, std::vector<Polity>& polities);

    static int CreateOrRefreshTreaty(std::vector<VassalTreaty>& treaties, Turn turn, PolityId overlord_id,
                                     PolityId subject_id, int source_occupation_id, float strength);

    static void UpdateTreaties(Turn turn, std::vector<Polity>& polities,
                               const std::vector<DiplomacyRelation>& diplomacy_relations,
                               std::vector<VassalTreaty>& treaties);
};

[[nodiscard]] const VassalTreaty* ActiveVassalTreatyForPair(const std::vector<VassalTreaty>& treaties, PolityId a,
                                                            PolityId b);
[[nodiscard]] const VassalTreaty* ActiveVassalTreatyForSubject(const std::vector<VassalTreaty>& treaties,
                                                               PolityId subject_id);

} // namespace oikumene
