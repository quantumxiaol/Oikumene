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
