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
    src/app/simulation_controller.cpp
    src/ai/heuristic_decision_provider.cpp
    src/ai/remote_decision_provider.cpp
    src/core/simulation.cpp
    src/network/http_client.cpp
    src/sim/band.cpp
    src/sim/band_system.cpp
    src/sim/event.cpp
    src/sim/event_log.cpp
    src/sim/settlement.cpp
    src/sim/settlement_system.cpp
    src/sim/simulation_params.cpp
    src/world/biome.cpp
    src/world/improvement.cpp
    src/world/resource.cpp
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
    src/app/oikumene_app.cpp
    src/render/camera_controller.cpp
    src/render/color_palette.cpp
    src/render/map_layer.cpp
    src/render/map_renderer.cpp
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

add_executable(oikumene_decision_tests
    tests/test_decision_models.cpp
)
target_link_libraries(oikumene_decision_tests PRIVATE oikumene_core)

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

if(BUILD_TESTING)
    add_test(NAME oikumene_app_config_tests COMMAND oikumene_app_config_tests)
    add_test(NAME oikumene_simulation_controller_tests COMMAND oikumene_simulation_controller_tests)
    add_test(NAME oikumene_decision_tests COMMAND oikumene_decision_tests)
    add_test(NAME oikumene_world_generation_tests COMMAND oikumene_world_generation_tests)
    add_test(NAME oikumene_world_generation_balance_tests COMMAND oikumene_world_generation_balance_tests)
    add_test(NAME oikumene_band_settlement_tests COMMAND oikumene_band_settlement_tests)
    add_test(NAME oikumene_settlement_economy_tests COMMAND oikumene_settlement_economy_tests)
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
