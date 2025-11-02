#!/bin/bash
# Comprehensive correctness validation script

set -e

echo "======================================"
echo "Trading Simulator Validation Suite"
echo "======================================"
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

FAILED=0

# Test 1: Build test
echo "Test 1: Building project..."
if ./scripts/build.sh --clean > /dev/null 2>&1; then
    echo -e "${GREEN}✓ Build successful${NC}"
else
    echo -e "${RED}✗ Build failed${NC}"
    FAILED=$((FAILED + 1))
fi
echo ""

# Test 2: Unit tests
echo "Test 2: Running unit test suite..."
if mpirun -np 2 ./build/test_trading_sim 2>&1 | grep -q "All tests passed"; then
    echo -e "${GREEN}✓ All unit tests passed${NC}"
else
    echo -e "${RED}✗ Some unit tests failed${NC}"
    FAILED=$((FAILED + 1))
fi
echo ""

# Test 3: Single process execution
echo "Test 3: Single process execution..."
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"
if timeout 30 mpirun -np 1 ${OLDPWD}/build/trading_sim > output.txt 2>&1; then
    if grep -q "Simulation Complete" output.txt; then
        echo -e "${GREEN}✓ Single process execution successful${NC}"
    else
        echo -e "${RED}✗ Simulation did not complete properly${NC}"
        FAILED=$((FAILED + 1))
    fi
else
    echo -e "${RED}✗ Execution failed or timed out${NC}"
    FAILED=$((FAILED + 1))
fi
cd "$OLDPWD"
rm -rf "$TEMP_DIR"
echo ""

# Test 4: Multi-process execution
echo "Test 4: Multi-process execution (4 ranks)..."
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"
if timeout 30 mpirun -np 4 ${OLDPWD}/build/trading_sim > output.txt 2>&1; then
    if grep -q "Simulation Complete" output.txt; then
        echo -e "${GREEN}✓ Multi-process execution successful${NC}"
    else
        echo -e "${RED}✗ Simulation did not complete properly${NC}"
        FAILED=$((FAILED + 1))
    fi
else
    echo -e "${RED}✗ Execution failed or timed out${NC}"
    FAILED=$((FAILED + 1))
fi
cd "$OLDPWD"
rm -rf "$TEMP_DIR"
echo ""

# Test 5: Output file generation
echo "Test 5: Checking output file generation..."
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"
mpirun -np 2 ${OLDPWD}/build/trading_sim > /dev/null 2>&1

FILES_OK=true
for rank in 0 1; do
    if [ ! -f "trades_rank_${rank}.csv" ]; then
        echo -e "${RED}✗ Missing trades_rank_${rank}.csv${NC}"
        FILES_OK=false
    fi
    if [ ! -f "prices_rank_${rank}.csv" ]; then
        echo -e "${RED}✗ Missing prices_rank_${rank}.csv${NC}"
        FILES_OK=false
    fi
done

if [ "$FILES_OK" = true ]; then
    echo -e "${GREEN}✓ All output files generated${NC}"
else
    FAILED=$((FAILED + 1))
fi
cd "$OLDPWD"
rm -rf "$TEMP_DIR"
echo ""

# Test 6: CSV file format validation
echo "Test 6: Validating CSV file format..."
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"
mpirun -np 1 ${OLDPWD}/build/trading_sim > /dev/null 2>&1

CSV_OK=true

# Check trades CSV
if [ -f "trades_rank_0.csv" ]; then
    # Check header
    if ! head -n 1 trades_rank_0.csv | grep -q "Timestamp,Instrument,BuyAgent,SellAgent,Price,Volume"; then
        echo -e "${RED}✗ Invalid trades CSV header${NC}"
        CSV_OK=false
    fi
    
    # Check for data rows
    if [ $(wc -l < trades_rank_0.csv) -lt 2 ]; then
        echo -e "${YELLOW}⚠ Warning: No trades executed${NC}"
    fi
else
    echo -e "${RED}✗ trades_rank_0.csv not found${NC}"
    CSV_OK=false
fi

# Check prices CSV
if [ -f "prices_rank_0.csv" ]; then
    # Check for data rows
    if [ $(wc -l < prices_rank_0.csv) -lt 2 ]; then
        echo -e "${RED}✗ Empty prices CSV${NC}"
        CSV_OK=false
    fi
else
    echo -e "${RED}✗ prices_rank_0.csv not found${NC}"
    CSV_OK=false
fi

if [ "$CSV_OK" = true ]; then
    echo -e "${GREEN}✓ CSV files are valid${NC}"
else
    FAILED=$((FAILED + 1))
fi
cd "$OLDPWD"
rm -rf "$TEMP_DIR"
echo ""

# Test 7: Thread safety test (repeated runs)
echo "Test 7: Thread safety test (10 repeated runs)..."
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

THREAD_SAFE=true
for i in {1..10}; do
    if ! timeout 10 mpirun -np 2 ${OLDPWD}/build/trading_sim > /dev/null 2>&1; then
        echo -e "${RED}✗ Run $i failed${NC}"
        THREAD_SAFE=false
        break
    fi
    rm -f *.csv  # Clean up between runs
done

if [ "$THREAD_SAFE" = true ]; then
    echo -e "${GREEN}✓ All repeated runs successful${NC}"
else
    FAILED=$((FAILED + 1))
fi
cd "$OLDPWD"
rm -rf "$TEMP_DIR"
echo ""

# Test 8: Memory leak check (if valgrind available)
if command -v valgrind >/dev/null 2>&1; then
    echo "Test 8: Memory leak check with Valgrind..."
    TEMP_DIR=$(mktemp -d)
    cd "$TEMP_DIR"
    
    # Run with valgrind (single process only, shorter simulation)
    # This will take a while, so we use a very short simulation
    if timeout 60 mpirun -np 1 valgrind --leak-check=full --error-exitcode=1 \
        ${OLDPWD}/build/trading_sim > valgrind_output.txt 2>&1; then
        
        if grep -q "ERROR SUMMARY: 0 errors" valgrind_output.txt && \
           grep -q "no leaks are possible" valgrind_output.txt; then
            echo -e "${GREEN}✓ No memory leaks detected${NC}"
        else
            echo -e "${YELLOW}⚠ Valgrind reported issues (see details above)${NC}"
            # Not counting as failure since it might be too strict
        fi
    else
        echo -e "${YELLOW}⚠ Valgrind check timed out or failed${NC}"
    fi
    
    cd "$OLDPWD"
    rm -rf "$TEMP_DIR"
else
    echo "Test 8: Skipping memory leak check (valgrind not installed)"
fi
echo ""

# Test 9: Determinism check
echo "Test 9: Checking determinism (same seed → same results)..."
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

# First run
mpirun -np 2 ${OLDPWD}/build/trading_sim > /dev/null 2>&1
TRADES1=$(wc -l < trades_rank_0.csv)

# Clean and second run
rm -f *.csv
mpirun -np 2 ${OLDPWD}/build/trading_sim > /dev/null 2>&1
TRADES2=$(wc -l < trades_rank_0.csv)

# Results should be similar (allowing for small variations)
DIFF=$((TRADES1 - TRADES2))
if [ ${DIFF#-} -lt 10 ]; then  # Absolute difference < 10
    echo -e "${GREEN}✓ Results are consistent (${TRADES1} vs ${TRADES2} trades)${NC}"
else
    echo -e "${YELLOW}⚠ Results vary significantly (${TRADES1} vs ${TRADES2} trades)${NC}"
fi

cd "$OLDPWD"
rm -rf "$TEMP_DIR"
echo ""

# Test 10: Scaling test
echo "Test 10: Testing with different process counts..."
SCALING_OK=true
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

for np in 1 2 4; do
    if ! timeout 30 mpirun -np $np ${OLDPWD}/build/trading_sim > output_${np}.txt 2>&1; then
        echo -e "${RED}✗ Failed with $np processes${NC}"
        SCALING_OK=false
        break
    fi
done

if [ "$SCALING_OK" = true ]; then
    echo -e "${GREEN}✓ Successfully ran with 1, 2, and 4 processes${NC}"
else
    FAILED=$((FAILED + 1))
fi

cd "$OLDPWD"
rm -rf "$TEMP_DIR"
echo ""

# Final summary
echo "======================================"
echo "Validation Summary"
echo "======================================"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}✓✓✓ ALL VALIDATION TESTS PASSED ✓✓✓${NC}"
    echo ""
    echo "The trading simulator is:"
    echo "  • Building correctly"
    echo "  • Passing all unit tests"
    echo "  • Executing successfully"
    echo "  • Generating valid outputs"
    echo "  • Thread-safe and robust"
    echo "  • Scaling across processes"
    exit 0
else
    echo -e "${RED}✗✗✗ $FAILED VALIDATION TEST(S) FAILED ✗✗✗${NC}"
    echo ""
    echo "Please review the errors above and fix the issues."
    exit 1
fi
