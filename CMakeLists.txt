cmake_minimum_required(VERSION 3.16)
project(BlockGameAI VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optimization flags for release builds
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Profiling build type (optimized + debug symbols for perf/Instruments)
set(CMAKE_CXX_FLAGS_PROFILE "-O3 -march=native -g -fno-omit-frame-pointer -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "")

# gprof profiling build type (instrumented)
set(CMAKE_CXX_FLAGS_GPROF "-O2 -pg -g -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_GPROF "-pg")

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Core game library
add_library(game_core STATIC
    src/board.cpp
    src/pieces.cpp
    src/game.cpp
)
target_include_directories(game_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Terminal game executable
add_executable(block_game src/main.cpp src/terminal_ui.cpp)
target_link_libraries(block_game PRIVATE game_core)

# Simulator executable
add_executable(simulator src/simulator.cpp)
target_link_libraries(simulator PRIVATE game_core)

# Perft executable
add_executable(perft src/perft.cpp)
target_link_libraries(perft PRIVATE game_core)
