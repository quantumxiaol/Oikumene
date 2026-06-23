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
    src/ai/heuristic_decision_provider.cpp
    src/ai/remote_decision_provider.cpp
    src/core/simulation.cpp
    src/network/http_client.cpp
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

add_executable(oikumene_tests
    tests/test_decision_models.cpp
)
target_link_libraries(oikumene_tests PRIVATE oikumene_core)

if(BUILD_TESTING)
    add_test(NAME oikumene_tests COMMAND oikumene_tests)
endif()
