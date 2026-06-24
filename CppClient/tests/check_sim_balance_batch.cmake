if(NOT DEFINED EXE)
    message(FATAL_ERROR "EXE is required")
endif()

if(NOT DEFINED OUT)
    message(FATAL_ERROR "OUT is required")
endif()

file(REMOVE_RECURSE "${OUT}")

execute_process(
    COMMAND "${EXE}" --start-seed 0 --count 4 --width 80 --height 56 --bands 8 --turns 80 --out "${OUT}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "oikumene_sim_balance_batch failed\nstdout:\n${stdout}\nstderr:\n${stderr}")
endif()

foreach(path summary.csv summary.json)
    if(NOT EXISTS "${OUT}/${path}")
        message(FATAL_ERROR "missing ${OUT}/${path}")
    endif()
endforeach()

file(READ "${OUT}/summary.json" summary)
foreach(token
    "\"aggregate\""
    "\"rows\""
    "\"mean_total_population\""
    "\"mean_farms\""
    "\"mean_pastures\""
    "\"mean_food_output_consumption_ratio\""
    "\"mean_farm_share_of_worked_tiles\""
    "\"mean_wood_output\""
    "\"mean_polities\""
    "\"mean_controlled_land_ratio\""
    "\"mean_contested_tiles\""
    "\"mean_largest_polity_population\""
    "\"mean_polity_food_income\""
    "\"mean_admin_load\""
    "\"mean_admin_capacity\""
    "\"mean_overextension\""
    "\"mean_stability\""
    "\"mean_control_maintenance\""
    "\"mean_unlocked_techs\""
    "\"mean_knowledge_income\""
    "\"pottery_unlock_rate\""
    "\"mining_unlock_rate\""
    "\"roads_unlock_rate\""
    "\"administration_unlock_rate\""
    "\"mean_ore_income\""
    "\"mean_tool_efficiency\""
    "\"mean_military_potential\""
    "\"mean_routes\""
    "\"mean_active_trades\""
    "\"mean_route_tiles\""
    "\"mean_admin_distance_cost\""
    "\"mean_admin_distance_saving\""
    "\"mean_connected_mine_potential\""
    "\"mean_active_connected_mines\""
    "\"mean_connected_ore_income\""
    "\"mean_unconnected_ore_income\""
    "\"mean_trade_profit\""
    "\"mean_trade_complementarity\""
    "\"mean_trade_route_cost\""
    "\"mean_trade_route_efficiency\""
    "\"mean_diplomacy_relations\""
    "\"mean_friendly_relations\""
    "\"mean_competitive_relations\""
    "\"mean_blockade_risk_relations\""
    "\"mean_friendship\""
    "\"mean_competition\""
    "\"mean_blockade_tendency\""
)
    string(FIND "${summary}" "${token}" token_index)
    if(token_index EQUAL -1)
        message(FATAL_ERROR "summary.json is missing ${token}")
    endif()
endforeach()

set(OUT_OFF "${OUT}_routes_off")
file(REMOVE_RECURSE "${OUT_OFF}")

execute_process(
    COMMAND "${EXE}" --start-seed 0 --count 4 --width 80 --height 56 --bands 8 --turns 80 --disable-routes --out "${OUT_OFF}"
    RESULT_VARIABLE off_result
    OUTPUT_VARIABLE off_stdout
    ERROR_VARIABLE off_stderr
)

if(NOT off_result EQUAL 0)
    message(FATAL_ERROR "oikumene_sim_balance_batch --disable-routes failed\nstdout:\n${off_stdout}\nstderr:\n${off_stderr}")
endif()

file(READ "${OUT_OFF}/summary.json" off_summary)
foreach(token
    "\"routes_enabled\": false"
    "\"mean_routes\""
    "\"mean_admin_distance_saving\""
)
    string(FIND "${off_summary}" "${token}" token_index)
    if(token_index EQUAL -1)
        message(FATAL_ERROR "routes-off summary.json is missing ${token}")
    endif()
endforeach()
