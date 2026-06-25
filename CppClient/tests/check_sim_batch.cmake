if(NOT DEFINED EXE)
    message(FATAL_ERROR "EXE is required")
endif()

if(NOT DEFINED OUT)
    message(FATAL_ERROR "OUT is required")
endif()

file(REMOVE_RECURSE "${OUT}")

execute_process(
    COMMAND "${EXE}" --seed 42 --width 80 --height 56 --bands 8 --turns 120 --sample-every 40 --out "${OUT}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "oikumene_sim_batch failed\nstdout:\n${stdout}\nstderr:\n${stderr}")
endif()

foreach(path summary.json final_state.json events.jsonl world_report.json states.jsonl)
    if(NOT EXISTS "${OUT}/${path}")
        message(FATAL_ERROR "missing ${OUT}/${path}")
    endif()
endforeach()

file(READ "${OUT}/summary.json" summary)
foreach(token
        "\"settlements\""
        "\"camps\""
        "\"villages\""
        "\"total_population\""
        "\"average_settlement_score\""
        "\"farm_count\""
        "\"lumbercamp_count\""
        "\"worked_tile_count\""
        "\"total_food_output_last_turn\""
        "\"average_carrying_capacity\""
        "\"polities\""
        "\"routes_enabled\""
        "\"routes\""
        "\"active_trades\""
        "\"total_trade_profit\""
        "\"diplomacy_relations\""
        "\"friendly_relations\""
        "\"competitive_relations\""
        "\"blockade_risk_relations\""
        "\"average_diplomatic_grievance\""
        "\"average_diplomatic_restraint\""
        "\"war_pressure_candidates\""
        "\"high_war_pressure_candidates\""
        "\"average_war_roi\""
        "\"max_declaration_pressure\""
        "\"average_trade_conflict_weight\""
        "\"average_friendly_penalty\""
        "\"average_grievance_pressure\""
        "\"average_restraint_pressure\""
        "\"war_target_candidates\""
        "\"high_war_target_candidates\""
        "\"average_war_target_roi\""
        "\"max_war_target_score\""
        "\"average_campaign_cost\""
        "\"war_campaigns\""
        "\"active_wars\""
        "\"occupied_wars\""
        "\"withdrawn_wars\""
        "\"occupations\""
        "\"active_occupations\""
        "\"vassalized_occupations\""
        "\"vassal_treaties\""
        "\"active_vassal_treaties\""
        "\"average_vassal_liberty_desire\""
        "\"average_active_occupation_unrest\""
        "\"war_population_lost\""
        "\"war_food_spent\""
        "\"route_tile_count\""
        "\"controlled_land_ratio\""
        "\"average_admin_load\""
        "\"average_stability\""
        "\"average_unlocked_techs\""
        "\"average_knowledge_income\""
        "\"mining_unlock_rate\"")
    string(FIND "${summary}" "${token}" token_index)
    if(token_index EQUAL -1)
        message(FATAL_ERROR "summary.json is missing ${token}")
    endif()
endforeach()

file(READ "${OUT}/final_state.json" final_state)
foreach(token
        "\"last_decision_reason\""
        "\"local_food_output_last_turn\""
        "\"upgrade_readiness\""
        "\"carrying_capacity\""
        "\"improved_tiles\""
        "\"routes\""
        "\"trades\""
        "\"diplomacy_relations\""
        "\"grievance_a_to_b\""
        "\"active_vassal_treaty_id\""
        "\"treaty_liberty_desire\""
        "\"last_incident\""
        "\"war_pressures\""
        "\"grievance_pressure\""
        "\"restraint_pressure\""
        "\"war_targets\""
        "\"campaign_cost\""
        "\"occupation_cost\""
        "\"wars\""
        "\"occupations\""
        "\"vassal_treaties\""
        "\"overlord_polity_id\""
        "\"vassal_liberty_desire\""
        "\"occupation_load\""
        "\"vassal_count\""
        "\"route_tiles\""
        "\"polity_id\""
        "\"polities\""
        "\"admin_load\""
        "\"overextension\""
        "\"budget\""
        "\"research\""
        "\"unlocked_techs\""
        "\"active_effects\""
        "\"military_potential\""
        "\"tool_efficiency\""
        "\"connected_mine_potential\""
        "\"active_connected_mines\""
        "\"trade_profit\"")
    string(FIND "${final_state}" "${token}" token_index)
    if(token_index EQUAL -1)
        message(FATAL_ERROR "final_state.json is missing ${token}")
    endif()
endforeach()
