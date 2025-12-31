cmake_minimum_required(VERSION 3.16)
project(BlockGameAI VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optimization flags for release builds
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

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
