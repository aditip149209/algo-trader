#!/bin/bash
# Build automation script for trading simulator

set -e  # Exit on error

echo "=== Trading Simulator Build Script ==="

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Check for required tools
echo "Checking prerequisites..."

command -v cmake >/dev/null 2>&1 || { 
    echo -e "${RED}ERROR: cmake not found${NC}"
    exit 1
}

command -v mpirun >/dev/null 2>&1 || { 
    echo -e "${RED}ERROR: mpirun not found${NC}"
    exit 1
}

echo -e "${GREEN}✓ All prerequisites found${NC}"

# Parse arguments
BUILD_TYPE="Release"
CLEAN=false
RUN_TESTS=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --test)
            RUN_TESTS=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--debug] [--clean] [--test]"
            exit 1
            ;;
    esac
done

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo "Cleaning build directory..."
    rm -rf build
fi

# Create build directory
mkdir -p build
cd build

# Configure
echo "Configuring with CMake ($BUILD_TYPE mode)..."
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

# Build
echo "Building..."
make -j$(nproc)

echo -e "${GREEN}✓ Build successful${NC}"

# Run tests if requested
if [ "$RUN_TESTS" = true ]; then
    echo ""
    echo "Running test suite..."
    mpirun -np 2 ./test_trading_sim
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ All tests passed${NC}"
    else
        echo -e "${RED}✗ Some tests failed${NC}"
        exit 1
    fi
fi

echo ""
echo "=== Build Complete ==="
echo "Executable: build/trading_sim"
echo "Test suite: build/test_trading_sim"
echo ""
echo "To run: mpirun -np 4 ./build/trading_sim"
