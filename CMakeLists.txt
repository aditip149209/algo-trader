cmake_minimum_required(VERSION 3.10)
project(TradingSimulator CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
if (WIN32)
    # Try MS-MPI (Microsoft MPI) on Windows
    set(MSMPI_INC $ENV{MSMPI_INC})
    if (NOT MSMPI_INC)
        set(MSMPI_INC "C:/Program Files (x86)/Microsoft SDKs/MPI/Include")
    endif()
    set(MSMPI_LIB64 $ENV{MSMPI_LIB64})
    if (NOT MSMPI_LIB64)
        set(MSMPI_LIB64 "C:/Program Files (x86)/Microsoft SDKs/MPI/Lib/x64")
    endif()

    if (EXISTS "${MSMPI_INC}/mpi.h" AND EXISTS "${MSMPI_LIB64}/msmpi.lib")
        set(MPI_FOUND TRUE)
        set(MPI_CXX_INCLUDE_DIRS "${MSMPI_INC}")
        set(MPI_CXX_LIBRARIES "${MSMPI_LIB64}/msmpi.lib")
        message(STATUS "MS-MPI detected at: ${MSMPI_INC} and ${MSMPI_LIB64}")
    else()
        message(FATAL_ERROR "MS-MPI not found. Please install MS-MPI SDK and ensure MSMPI_INC/MSMPI_LIB64 are set.")
    endif()
else()
    find_package(MPI REQUIRED)
endif()

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
if (MSVC)
    target_compile_options(trading_sim PRIVATE /O2 /W4)
else()
    target_compile_options(trading_sim PRIVATE -Wall -Wextra -O3 -march=native)
endif()

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

if (MSVC)
    target_compile_options(test_trading_sim PRIVATE /O2 /W4)
else()
    target_compile_options(test_trading_sim PRIVATE -Wall -Wextra -O3 -march=native)
endif()

# Installation
install(TARGETS trading_sim DESTINATION bin)

# Print configuration info
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "MPI Found: ${MPI_FOUND}")
message(STATUS "OpenMP Found: ${OpenMP_FOUND}")
