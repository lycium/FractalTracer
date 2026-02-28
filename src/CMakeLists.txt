cmake_minimum_required(VERSION 3.13)

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()

project(FractalTracer)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Wextra -pedantic)
add_compile_options(-march=native)

include(libs.cmake)

find_package(OpenMP)
find_package(Threads)

# Fetch nlohmann/json (used by both CLI and UI for scene serialization)
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# CLI batch renderer
add_executable(FractalTracer demo/main.cpp ui/SceneSerializer.cpp ui/Timeline.cpp)
target_include_directories(FractalTracer PUBLIC .)
target_link_libraries(FractalTracer
    Threads::Threads OpenMP::OpenMP_CXX
    TracerLib
    nlohmann_json::nlohmann_json
    )

# Interactive UI (SDL2 + Dear ImGui)
find_package(SDL2 QUIET)
if(SDL2_FOUND)
    # Fetch Dear ImGui
    include(FetchContent)
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.8
    )
    FetchContent_MakeAvailable(imgui)

    # Build Dear ImGui as a static library
    add_library(imgui_lib STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp
    )
    target_include_directories(imgui_lib PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui_lib PUBLIC SDL2::SDL2)

    add_executable(FractalTracerUI
        ui/main_ui.cpp
        ui/RenderController.cpp
        ui/ImGuiPanels.cpp
        ui/InteractiveCamera.cpp
        ui/SceneSerializer.cpp
        ui/Timeline.cpp
        ui/TimelinePanel.cpp
        ui/AnimationRenderer.cpp
    )
    target_include_directories(FractalTracerUI PUBLIC .)
    target_link_libraries(FractalTracerUI
        TracerLib
        Threads::Threads OpenMP::OpenMP_CXX
        SDL2::SDL2
        imgui_lib
        nlohmann_json::nlohmann_json
    )
else()
    message(STATUS "SDL2 not found, skipping FractalTracerUI target")
endif()
