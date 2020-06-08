cmake_minimum_required(VERSION 3.13)

project(FractalTracer)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

include(libs.cmake)

find_package(OpenMP)
find_package(Threads)

add_executable(FractalTracer demo/main.cpp)
target_include_directories(FractalTracer PUBLIC .)
target_link_libraries(FractalTracer
    Threads::Threads OpenMP::OpenMP_CXX
    TracerLib
    )
