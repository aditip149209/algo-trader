@echo off
REM Build automation script for trading simulator (Windows)

echo === Trading Simulator Build Script ===

REM Check for required tools
echo Checking prerequisites...

where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: cmake not found
    exit /b 1
)

where mpiexec >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: mpiexec not found
    echo HINT: Install Microsoft MPI ^(MS-MPI^) Redistributable and SDK.
    echo URL: https://learn.microsoft.com/en-us/message-passing-interface/microsoft-mpi
    exit /b 1
)

REM Check for MS-MPI SDK (headers/libs) - runtime alone is not enough for building
if "%MSMPI_INC%"=="" (
    set "_MSMPI_INC_DEFAULT=C:\Program Files (x86)\Microsoft SDKs\MPI\Include"
)
if "%MSMPI_LIB64%"=="" (
    set "_MSMPI_LIB64_DEFAULT=C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64"
)

if not defined MSMPI_INC if not exist "%_MSMPI_INC_DEFAULT%\mpi.h" (
    echo ERROR: MS-MPI SDK headers not found.
    echo HINT: Install the MS-MPI SDK so CMake can find mpi.h and msmpi.lib
    echo URL: https://learn.microsoft.com/en-us/message-passing-interface/microsoft-mpi
    echo If already installed, ensure MSMPI_INC and MSMPI_LIB64 environment variables are set.
    exit /b 1
)

echo Prerequisites found.

REM Parse arguments
set BUILD_TYPE=Release
set CLEAN=false
set RUN_TESTS=false

:parse_args
if "%1"=="" goto end_parse
if "%1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if "%1"=="--clean" (
    set CLEAN=true
    shift
    goto parse_args
)
if "%1"=="--test" (
    set RUN_TESTS=true
    shift
    goto parse_args
)
echo Unknown option: %1
echo Usage: %0 [--debug] [--clean] [--test]
exit /b 1

:end_parse

REM Clean if requested
if "%CLEAN%"=="true" (
    echo Cleaning build directory...
    rmdir /s /q build 2>nul
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure
echo Configuring with CMake (%BUILD_TYPE% mode)...
REM Provide search paths to CMake so FindMPI can locate MS-MPI
if defined MSMPI_INC set "CMAKE_INCLUDE_PATH=%MSMPI_INC%"
if defined MSMPI_LIB64 set "CMAKE_LIBRARY_PATH=%MSMPI_LIB64%"
if not defined MSMPI_INC if exist "%_MSMPI_INC_DEFAULT%\mpi.h" set "CMAKE_INCLUDE_PATH=%_MSMPI_INC_DEFAULT%"
if not defined MSMPI_LIB64 if exist "%_MSMPI_LIB64_DEFAULT%\msmpi.lib" set "CMAKE_LIBRARY_PATH=%_MSMPI_LIB64_DEFAULT%"

cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

REM Build
echo Building...
cmake --build . --config %BUILD_TYPE%

if %errorlevel% neq 0 (
    echo Build failed.
    exit /b 1
)

echo Build successful.

REM Run tests if requested
if "%RUN_TESTS%"=="true" (
    echo.
    echo Running test suite...
    mpiexec -n 2 %BUILD_TYPE%\test_trading_sim.exe
    
    if %errorlevel% equ 0 (
        echo All tests passed.
    ) else (
        echo Some tests failed.
        exit /b 1
    )
)

echo.
echo === Build Complete ===
echo Executable: %BUILD_TYPE%\trading_sim.exe
echo Test suite: %BUILD_TYPE%\test_trading_sim.exe
echo.
echo To run: mpiexec -n 4 %BUILD_TYPE%\trading_sim.exe