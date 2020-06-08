cmake_minimum_required(VERSION 3.13)

project(FractalTracer)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

find_package(OpenMP)
find_package(Threads)

include(libs.cmake)

add_executable(FractalTracer demo/main.cpp)
target_link_libraries(FractalTracer
    Threads::Threads
    OpenMP::OpenMP_CXX

    formulas
    maths
    util
    renderer
    scene_objects
    )
