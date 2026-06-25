cmake_minimum_required(VERSION 3.20)

project(OikumeneCppClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")
endif()

include(CTest)

find_package(raylib CONFIG QUIET)
find_package(nlohmann_json CONFIG QUIET)

if(NOT raylib_FOUND)
    find_path(RAYLIB_INCLUDE_DIR raylib.h
        PATHS /opt/homebrew/include /usr/local/include
    )
    find_library(RAYLIB_LIBRARY raylib
        PATHS /opt/homebrew/lib /usr/local/lib
    )
    if(NOT RAYLIB_INCLUDE_DIR OR NOT RAYLIB_LIBRARY)
        message(FATAL_ERROR "raylib was not found. Install it with brew install raylib or set RAYLIB_INCLUDE_DIR/RAYLIB_LIBRARY.")
    endif()
endif()

if(NOT nlohmann_json_FOUND)
    find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp
        PATHS /opt/homebrew/include /usr/local/include
    )
    if(NOT NLOHMANN_JSON_INCLUDE_DIR)
        message(FATAL_ERROR "nlohmann/json.hpp was not found. Install nlohmann-json or add it to include paths.")
    endif()
endif()

include(cmake/ThirdParty.cmake)

add_library(oikumene_core STATIC
    src/app/app_config.cpp
    src/app/command_line.cpp
    src/app/input_policy.cpp
    src/app/simulation_controller.cpp
    src/ai/heuristic_decision_provider.cpp
    src/ai/remote_decision_provider.cpp
    src/ai/strategic_report_actions.cpp
    src/ai/strategic_report.cpp
    src/ai/strategic_report_helpers.cpp
    src/ai/strategic_report_summary.cpp
    src/core/simulation.cpp
    src/io/simulation_json.cpp
    src/io/simulation_snapshot_json.cpp
    src/network/http_client.cpp
    src/sim/band.cpp
    src/sim/band_system.cpp
    src/sim/control_field.cpp
    src/sim/diplomacy.cpp
    src/sim/diplomacy_system.cpp
    src/sim/event.cpp
    src/sim/event_log.cpp
    src/sim/occupation.cpp
    src/sim/occupation_system.cpp
    src/sim/pathfinding.cpp
    src/sim/polity.cpp
    src/sim/polity_system.cpp
    src/sim/route.cpp
    src/sim/route_audit.cpp
    src/sim/route_planner.cpp
    src/sim/route_system.cpp
    src/sim/settlement.cpp
    src/sim/settlement_improvement.cpp
    src/sim/settlement_production.cpp
    src/sim/settlement_system.cpp
    src/sim/settlement_work.cpp
    src/sim/simulation_conflict_metrics.cpp
    src/sim/simulation_metrics.cpp
    src/sim/simulation_params.cpp
    src/sim/simulation_polity_metrics.cpp
    src/sim/tech_effects.cpp
    src/sim/technology.cpp
    src/sim/technology_system.cpp
    src/sim/trade.cpp
    src/sim/trade_planner.cpp
    src/sim/trade_system.cpp
    src/sim/vassal_system.cpp
    src/sim/vassal_treaty.cpp
    src/sim/war.cpp
    src/sim/war_campaign.cpp
    src/sim/war_planner.cpp
    src/sim/war_system.cpp
    src/sim/war_target.cpp
    src/sim/war_target_planner.cpp
    src/world/biome.cpp
    src/world/improvement.cpp
    src/world/resource.cpp
    src/world/route_kind.cpp
    src/world/world.cpp
    src/world/world_generation_report.cpp
    src/world/world_generation_params.cpp
    src/world/world_generator.cpp
)

target_include_directories(oikumene_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(nlohmann_json_FOUND)
    target_link_libraries(oikumene_core PUBLIC nlohmann_json::nlohmann_json)
else()
    target_include_directories(oikumene_core PUBLIC ${NLOHMANN_JSON_INCLUDE_DIR})
endif()

if(WIN32)
    target_link_libraries(oikumene_core PRIVATE ws2_32)
endif()

if(TARGET oikumene_fastnoise)
    target_link_libraries(oikumene_core PUBLIC oikumene_fastnoise)
    target_compile_definitions(oikumene_core PUBLIC OIKUMENE_HAS_FASTNOISELITE=1)
else()
    target_compile_definitions(oikumene_core PUBLIC OIKUMENE_HAS_FASTNOISELITE=0)
endif()

add_executable(oikumene_app
    src/main.cpp
    src/app/input_controller.cpp
    src/app/oikumene_app.cpp
    src/io/export_service.cpp
    src/render/camera_controller.cpp
    src/render/color_palette.cpp
    src/render/map_layer.cpp
    src/render/map_renderer.cpp
    src/render/selection.cpp
    src/render/symbol_registry.cpp
    src/ui/details_panel.cpp
    src/ui/event_log_panel.cpp
    src/ui/help_panel.cpp
    src/ui/hud_panel.cpp
    src/ui/legend_panel.cpp
    src/ui/panel_layout.cpp
    src/ui/playback_bar.cpp
    src/ui/view_helpers.cpp
)

target_link_libraries(oikumene_app PRIVATE oikumene_core)

if(TARGET oikumene_imgui)
    target_link_libraries(oikumene_app PRIVATE oikumene_imgui)
    target_compile_definitions(oikumene_app PRIVATE OIKUMENE_HAS_IMGUI=1)
else()
    target_compile_definitions(oikumene_app PRIVATE OIKUMENE_HAS_IMGUI=0)
endif()

if(raylib_FOUND)
    target_link_libraries(oikumene_app PRIVATE raylib)
else()
    target_include_directories(oikumene_app PRIVATE ${RAYLIB_INCLUDE_DIR})
    target_link_libraries(oikumene_app PRIVATE ${RAYLIB_LIBRARY})
endif()

if(APPLE)
    target_link_libraries(oikumene_app PRIVATE
        "-framework IOKit"
        "-framework Cocoa"
        "-framework OpenGL"
    )
endif()

add_executable(oikumene_worldgen_batch
    tools/worldgen_batch.cpp
)
target_link_libraries(oikumene_worldgen_batch PRIVATE oikumene_core)

add_executable(oikumene_sim_batch
    tools/sim_batch.cpp
)
target_link_libraries(oikumene_sim_batch PRIVATE oikumene_core)

add_executable(oikumene_sim_balance_batch
    tools/sim_balance_batch.cpp
    tools/sim_balance_metrics.cpp
    tools/sim_balance_output.cpp
)
target_link_libraries(oikumene_sim_balance_batch PRIVATE oikumene_core)

add_executable(oikumene_app_config_tests
    tests/test_app_config.cpp
)
target_link_libraries(oikumene_app_config_tests PRIVATE oikumene_core)

add_executable(oikumene_simulation_controller_tests
    tests/test_simulation_controller.cpp
)
target_link_libraries(oikumene_simulation_controller_tests PRIVATE oikumene_core)

add_executable(oikumene_input_policy_tests
    tests/test_input_policy.cpp
)
target_link_libraries(oikumene_input_policy_tests PRIVATE oikumene_core)

add_executable(oikumene_decision_tests
    tests/test_decision_models.cpp
)
target_link_libraries(oikumene_decision_tests PRIVATE oikumene_core)

add_executable(oikumene_strategic_report_tests
    tests/test_strategic_report.cpp
)
target_link_libraries(oikumene_strategic_report_tests PRIVATE oikumene_core)

add_executable(oikumene_world_generation_tests
    tests/test_world_generation.cpp
)
target_link_libraries(oikumene_world_generation_tests PRIVATE oikumene_core)

add_executable(oikumene_world_generation_balance_tests
    tests/test_world_generation_balance.cpp
)
target_link_libraries(oikumene_world_generation_balance_tests PRIVATE oikumene_core)

add_executable(oikumene_band_settlement_tests
    tests/test_band_settlement.cpp
)
target_link_libraries(oikumene_band_settlement_tests PRIVATE oikumene_core)

add_executable(oikumene_settlement_economy_tests
    tests/test_settlement_economy.cpp
)
target_link_libraries(oikumene_settlement_economy_tests PRIVATE oikumene_core)

add_executable(oikumene_camera_controller_tests
    tests/test_camera_controller.cpp
    src/render/camera_controller.cpp
)
target_link_libraries(oikumene_camera_controller_tests PRIVATE oikumene_core)

add_executable(oikumene_selection_tests
    tests/test_selection.cpp
    src/render/selection.cpp
)
target_link_libraries(oikumene_selection_tests PRIVATE oikumene_core)

add_executable(oikumene_polity_tests
    tests/test_polity.cpp
)
target_link_libraries(oikumene_polity_tests PRIVATE oikumene_core)

add_executable(oikumene_control_field_tests
    tests/test_control_field.cpp
)
target_link_libraries(oikumene_control_field_tests PRIVATE oikumene_core)

add_executable(oikumene_polity_economy_tests
    tests/test_polity_economy.cpp
)
target_link_libraries(oikumene_polity_economy_tests PRIVATE oikumene_core)

add_executable(oikumene_symbol_registry_tests
    tests/test_symbol_registry.cpp
    src/render/color_palette.cpp
    src/render/symbol_registry.cpp
)
target_link_libraries(oikumene_symbol_registry_tests PRIVATE oikumene_core)

add_executable(oikumene_technology_tests
    tests/test_technology.cpp
)
target_link_libraries(oikumene_technology_tests PRIVATE oikumene_core)

add_executable(oikumene_tech_effects_tests
    tests/test_tech_effects.cpp
)
target_link_libraries(oikumene_tech_effects_tests PRIVATE oikumene_core)

add_executable(oikumene_pathfinding_tests
    tests/test_pathfinding.cpp
)
target_link_libraries(oikumene_pathfinding_tests PRIVATE oikumene_core)

add_executable(oikumene_route_network_tests
    tests/test_route_network.cpp
)
target_link_libraries(oikumene_route_network_tests PRIVATE oikumene_core)

add_executable(oikumene_route_effects_tests
    tests/test_route_effects.cpp
)
target_link_libraries(oikumene_route_effects_tests PRIVATE oikumene_core)

add_executable(oikumene_trade_system_tests
    tests/test_trade_system.cpp
)
target_link_libraries(oikumene_trade_system_tests PRIVATE oikumene_core)

add_executable(oikumene_diplomacy_system_tests
    tests/test_diplomacy_system.cpp
)
target_link_libraries(oikumene_diplomacy_system_tests PRIVATE oikumene_core)

add_executable(oikumene_war_planner_tests
    tests/test_war_planner.cpp
)
target_link_libraries(oikumene_war_planner_tests PRIVATE oikumene_core)

add_executable(oikumene_war_target_planner_tests
    tests/test_war_target_planner.cpp
)
target_link_libraries(oikumene_war_target_planner_tests PRIVATE oikumene_core)

add_executable(oikumene_war_system_tests
    tests/test_war_system.cpp
)
target_link_libraries(oikumene_war_system_tests PRIVATE oikumene_core)

add_executable(oikumene_occupation_system_tests
    tests/test_occupation_system.cpp
)
target_link_libraries(oikumene_occupation_system_tests PRIVATE oikumene_core)

add_executable(oikumene_vassal_system_tests
    tests/test_vassal_system.cpp
)
target_link_libraries(oikumene_vassal_system_tests PRIVATE oikumene_core)

foreach(target_name oikumene_camera_controller_tests oikumene_selection_tests oikumene_symbol_registry_tests)
    if(raylib_FOUND)
        target_link_libraries(${target_name} PRIVATE raylib)
    else()
        target_include_directories(${target_name} PRIVATE ${RAYLIB_INCLUDE_DIR})
        target_link_libraries(${target_name} PRIVATE ${RAYLIB_LIBRARY})
    endif()
endforeach()

if(BUILD_TESTING)
    add_test(NAME oikumene_app_config_tests COMMAND oikumene_app_config_tests)
    add_test(NAME oikumene_simulation_controller_tests COMMAND oikumene_simulation_controller_tests)
    add_test(NAME oikumene_input_policy_tests COMMAND oikumene_input_policy_tests)
    add_test(NAME oikumene_decision_tests COMMAND oikumene_decision_tests)
    add_test(NAME oikumene_strategic_report_tests COMMAND oikumene_strategic_report_tests)
    add_test(NAME oikumene_world_generation_tests COMMAND oikumene_world_generation_tests)
    add_test(NAME oikumene_world_generation_balance_tests COMMAND oikumene_world_generation_balance_tests)
    add_test(NAME oikumene_band_settlement_tests COMMAND oikumene_band_settlement_tests)
    add_test(NAME oikumene_settlement_economy_tests COMMAND oikumene_settlement_economy_tests)
    add_test(NAME oikumene_camera_controller_tests COMMAND oikumene_camera_controller_tests)
    add_test(NAME oikumene_selection_tests COMMAND oikumene_selection_tests)
    add_test(NAME oikumene_polity_tests COMMAND oikumene_polity_tests)
    add_test(NAME oikumene_control_field_tests COMMAND oikumene_control_field_tests)
    add_test(NAME oikumene_polity_economy_tests COMMAND oikumene_polity_economy_tests)
    add_test(NAME oikumene_symbol_registry_tests COMMAND oikumene_symbol_registry_tests)
    add_test(NAME oikumene_technology_tests COMMAND oikumene_technology_tests)
    add_test(NAME oikumene_tech_effects_tests COMMAND oikumene_tech_effects_tests)
    add_test(NAME oikumene_pathfinding_tests COMMAND oikumene_pathfinding_tests)
    add_test(NAME oikumene_route_network_tests COMMAND oikumene_route_network_tests)
    add_test(NAME oikumene_route_effects_tests COMMAND oikumene_route_effects_tests)
    add_test(NAME oikumene_trade_system_tests COMMAND oikumene_trade_system_tests)
    add_test(NAME oikumene_diplomacy_system_tests COMMAND oikumene_diplomacy_system_tests)
    add_test(NAME oikumene_war_planner_tests COMMAND oikumene_war_planner_tests)
    add_test(NAME oikumene_war_target_planner_tests COMMAND oikumene_war_target_planner_tests)
    add_test(NAME oikumene_war_system_tests COMMAND oikumene_war_system_tests)
    add_test(NAME oikumene_occupation_system_tests COMMAND oikumene_occupation_system_tests)
    add_test(NAME oikumene_vassal_system_tests COMMAND oikumene_vassal_system_tests)
    add_test(
        NAME oikumene_sim_batch_smoke
        COMMAND
            ${CMAKE_COMMAND}
            -DEXE=$<TARGET_FILE:oikumene_sim_batch>
            -DOUT=${CMAKE_CURRENT_BINARY_DIR}/sim_batch_smoke
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_sim_batch.cmake
    )
    add_test(
        NAME oikumene_sim_balance_batch_smoke
        COMMAND
            ${CMAKE_COMMAND}
            -DEXE=$<TARGET_FILE:oikumene_sim_balance_batch>
            -DOUT=${CMAKE_CURRENT_BINARY_DIR}/sim_balance_batch_smoke
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_sim_balance_batch.cmake
    )
endif()
