set(OIKUMENE_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

set(OIKUMENE_IMGUI_DIR "${OIKUMENE_THIRD_PARTY_DIR}/imgui")
set(OIKUMENE_RLIMGUI_DIR "${OIKUMENE_THIRD_PARTY_DIR}/rlImGui")
set(OIKUMENE_FASTNOISELITE_DIR "${OIKUMENE_THIRD_PARTY_DIR}/FastNoiseLite")

option(OIKUMENE_ENABLE_VENDOR_IMGUI "Build vendored Dear ImGui + rlImGui when present" ON)
option(OIKUMENE_ENABLE_VENDOR_FASTNOISELITE "Expose vendored FastNoiseLite when present" ON)

if(OIKUMENE_ENABLE_VENDOR_IMGUI)
    set(OIKUMENE_IMGUI_SOURCES
        "${OIKUMENE_IMGUI_DIR}/imgui.cpp"
        "${OIKUMENE_IMGUI_DIR}/imgui_draw.cpp"
        "${OIKUMENE_IMGUI_DIR}/imgui_tables.cpp"
        "${OIKUMENE_IMGUI_DIR}/imgui_widgets.cpp"
        "${OIKUMENE_RLIMGUI_DIR}/rlImGui.cpp"
    )

    set(OIKUMENE_IMGUI_HEADERS
        "${OIKUMENE_IMGUI_DIR}/imgui.h"
        "${OIKUMENE_RLIMGUI_DIR}/rlImGui.h"
    )

    set(OIKUMENE_IMGUI_READY TRUE)
    foreach(path IN LISTS OIKUMENE_IMGUI_SOURCES OIKUMENE_IMGUI_HEADERS)
        if(NOT EXISTS "${path}")
            set(OIKUMENE_IMGUI_READY FALSE)
        endif()
    endforeach()

    if(OIKUMENE_IMGUI_READY)
        add_library(oikumene_imgui STATIC ${OIKUMENE_IMGUI_SOURCES})
        target_include_directories(oikumene_imgui
            PUBLIC
                "${OIKUMENE_IMGUI_DIR}"
                "${OIKUMENE_RLIMGUI_DIR}"
        )

        if(TARGET raylib)
            target_link_libraries(oikumene_imgui PUBLIC raylib)
        else()
            target_include_directories(oikumene_imgui PUBLIC ${RAYLIB_INCLUDE_DIR})
            target_link_libraries(oikumene_imgui PUBLIC ${RAYLIB_LIBRARY})
        endif()

        message(STATUS "Oikumene: enabled vendored Dear ImGui + rlImGui")
    else()
        message(STATUS "Oikumene: vendored Dear ImGui + rlImGui not found; run scripts/vendor_third_party.py when UI panels are needed")
    endif()
endif()

if(OIKUMENE_ENABLE_VENDOR_FASTNOISELITE)
    if(EXISTS "${OIKUMENE_FASTNOISELITE_DIR}/Cpp/FastNoiseLite.h")
        add_library(oikumene_fastnoise INTERFACE)
        target_include_directories(oikumene_fastnoise
            INTERFACE
                "${OIKUMENE_FASTNOISELITE_DIR}/Cpp"
        )
        message(STATUS "Oikumene: enabled vendored FastNoiseLite")
    else()
        message(STATUS "Oikumene: vendored FastNoiseLite not found; run scripts/vendor_third_party.py before Phase 1 world generation")
    endif()
endif()
