cmake_minimum_required(VERSION 3.10)
project(TradingSimulator CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${MPI_CXX_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/main.cpp
    src/exchange.cpp
    src/agent.cpp
    src/marketdata.cpp
    src/utils.cpp
)

# Main executable
add_executable(trading_sim ${SOURCES})

# Link libraries
target_link_libraries(trading_sim
    ${MPI_CXX_LIBRARIES}
    OpenMP::OpenMP_CXX
)

# Compiler flags
target_compile_options(trading_sim PRIVATE
    -Wall
    -Wextra
    -O3
    -march=native
)

# Test executable
add_executable(test_trading_sim
    tests/test_suite.cpp
    src/exchange.cpp
    src/agent.cpp
    src/marketdata.cpp
    src/utils.cpp
)

target_link_libraries(test_trading_sim
    ${MPI_CXX_LIBRARIES}
    OpenMP::OpenMP_CXX
)

# Installation
install(TARGETS trading_sim DESTINATION bin)

# Print configuration info
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "MPI Found: ${MPI_FOUND}")
message(STATUS "OpenMP Found: ${OpenMP_FOUND}")
