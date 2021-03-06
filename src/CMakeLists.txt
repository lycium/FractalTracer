cmake_minimum_required(VERSION 3.13)

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()

project(FractalTracer)

set(CMAKE_CXX_STANDARD 17)

if(MSVC OR MSVC_IDE)
    message(FATAL_ERROR
        "To build this project with MSVC please use the solution in /FractalTracer_msvc2019"
        )
endif()

add_compile_options(-Wall -Wextra -pedantic)
add_compile_options(-march=native)

include(libs.cmake)

find_package(OpenMP)
find_package(Threads)

add_executable(FractalTracer demo/main.cpp)
target_include_directories(FractalTracer PUBLIC .)
target_link_libraries(FractalTracer
    Threads::Threads OpenMP::OpenMP_CXX
    TracerLib
    )
