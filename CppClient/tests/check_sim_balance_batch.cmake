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
)
    string(FIND "${summary}" "${token}" token_index)
    if(token_index EQUAL -1)
        message(FATAL_ERROR "summary.json is missing ${token}")
    endif()
endforeach()
