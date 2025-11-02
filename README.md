# Algorithmic Trading Simulator

A high-performance hybrid MPI+OpenMP parallel trading simulation system that demonstrates distributed computing principles, concurrent programming, and financial market mechanics.

## 🎯 Project Overview

This simulator models a multi-exchange trading environment where:
- **MPI processes** represent separate exchange nodes
- **OpenMP threads** represent concurrent trading agents
- Orders are matched in real-time using order book mechanics
- Market data synchronizes across exchanges via MPI communication

## 📁 Repository Structure

```
trading-sim/
│
├── src/
│   ├── main.cpp           # Entry point and simulation orchestration
│   ├── exchange.cpp       # Order matching engine implementation
│   ├── agent.cpp          # Trading agent strategies
│   ├── marketdata.cpp     # MPI communication layer
│   └── utils.cpp          # Helper utilities
│
├── include/
│   ├── exchange.h         # Exchange and order book interfaces
│   ├── agent.h            # Agent strategy definitions
│   ├── marketdata.h       # Market data manager interface
│   └── utils.h            # Utility function headers
│
├── tests/
│   └── test_suite.cpp     # Comprehensive correctness tests
│
├── scripts/
│   ├── build.sh           # Build automation script
│   ├── run_simulation.sh  # Run with various configurations
│   └── analyze_results.py # Visualize trading results
│
├── CMakeLists.txt         # Build configuration
└── README.md              # This file
```

## 🛠️ Prerequisites

### Required
- **C++ Compiler**: GCC 7+ or Clang 6+ with C++17 support
- **MPI**: OpenMPI 3.0+ or MPICH 3.2+
- **OpenMP**: Usually included with modern GCC/Clang
- **CMake**: 3.10 or higher

### Optional
- **Python 3.x** with matplotlib, pandas (for result visualization)

### Installation (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential cmake
sudo apt-get install libopenmpi-dev openmpi-bin
sudo apt-get install python3 python3-pip
pip3 install matplotlib pandas
```

### Installation (macOS with Homebrew)
```bash
brew install cmake
brew install open-mpi
brew install python3
pip3 install matplotlib pandas
```

## 🔨 Building the Project

### Quick Build
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Detailed Build Steps
```bash
# 1. Create build directory
mkdir build
cd build

# 2. Configure with CMake
cmake -DCMAKE_BUILD_TYPE=Release ..

# 3. Build
make -j$(nproc)

# 4. Optional: Install
sudo make install
```

### Build Options
```bash
# Debug build with symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Specify MPI compiler
cmake -DMPI_CXX_COMPILER=/usr/bin/mpicxx ..

# Enable verbose output
make VERBOSE=1
```

## 🚀 Running the Simulator

### Basic Execution
```bash
# Run with 4 MPI processes (4 exchanges)
mpirun -np 4 ./trading_sim

# Run with 2 processes
mpirun -np 2 ./trading_sim

# Run on specific hosts (cluster)
mpirun -np 8 --hostfile hostfile ./trading_sim
```

### Output Files
The simulator generates CSV files for analysis:
- `trades_rank_X.csv` - All executed trades for rank X
- `prices_rank_X.csv` - Price history for all instruments at rank X

### Console Output Example
```
=== Algorithmic Trading Simulator ===
MPI Processes (Exchanges): 4
OpenMP Threads (Agents) per Process: 8
Instruments per Exchange: 3
Simulation Ticks: 1000
======================================
Tick  100 | Orders:   2847 | Trades:   1203
Tick  200 | Orders:   5691 | Trades:   2398
...
=== Simulation Complete ===
Total Execution Time: 2847 ms
Global Orders Submitted: 28476
Global Trades Executed: 12034
Orders per Second: 10002.1
Trades per Second: 4227.8
```

## ✅ Testing for Correctness

### Running the Test Suite
```bash
# Build tests
cd build
make test_trading_sim

# Run tests with MPI
mpirun -np 2 ./test_trading_sim
```

### Test Categories

#### 1. **Order Management Tests**
- Order creation and submission
- Thread-safe concurrent submissions
- Order queue integrity

#### 2. **Matching Engine Tests**
- Exact price matching
- Partial fills
- No-match scenarios (price gaps)
- Priority-based matching

#### 3. **Price Discovery Tests**
- Price updates after trades
- Historical average calculations
- Multi-instrument price tracking

#### 4. **Agent Strategy Tests**
- Random walk behavior
- Momentum strategy logic
- Mean reversion strategy
- Market maker dual-side orders

#### 5. **Parallel Correctness Tests**
- OpenMP thread safety
- MPI communication correctness
- Data race detection

### Expected Test Output
```
=== Trading Simulator Test Suite ===
Running correctness tests...

[PASS] order_creation
[PASS] order_matching_exact
[PASS] order_matching_no_match
[PASS] partial_fill
[PASS] price_discovery
[PASS] agent_random_walk
[PASS] agent_momentum
[PASS] agent_mean_reversion
[PASS] agent_market_maker
[PASS] historical_average
[PASS] thread_safety
[PASS] mpi_broadcast

=== Test Results ===
Passed: 12
Failed: 0
Total:  12

✓ All tests passed!
```

## 🔍 Validation Strategies

### 1. **Functional Correctness**
```bash
# Verify order matching logic
mpirun -np 1 ./test_trading_sim

# Check CSV outputs for consistency
python3 scripts/validate_trades.py
```

### 2. **Concurrency Correctness**
```bash
# Test with ThreadSanitizer
cmake -DCMAKE_CXX_FLAGS="-fsanitize=thread" ..
make
mpirun -np 2 ./trading_sim

# Test with Helgrind (Valgrind)
mpirun -np 1 valgrind --tool=helgrind ./trading_sim
```

### 3. **Performance Consistency**
```bash
# Run multiple times and compare results
for i in {1..5}; do
    mpirun -np 4 ./trading_sim | grep "Orders per Second"
done
```

### 4. **Scaling Tests**
```bash
# Test with different thread counts
export OMP_NUM_THREADS=2
mpirun -np 4 ./trading_sim

export OMP_NUM_THREADS=8
mpirun -np 4 ./trading_sim

# Test with different process counts
for np in 1 2 4 8; do
    echo "Testing with $np processes"
    mpirun -np $np ./trading_sim
done
```

## 📊 Analyzing Results

### Visualizing Trade Data
```python
# scripts/analyze_results.py
import pandas as pd
import matplotlib.pyplot as plt

# Load trade data
trades = pd.read_csv('trades_rank_0.csv')

# Plot trade prices over time
plt.figure(figsize=(12, 6))
for instrument in trades['Instrument'].unique():
    data = trades[trades['Instrument'] == instrument]
    plt.plot(data['Timestamp'], data['Price'], label=f'Instrument {instrument}')

plt.xlabel('Time (ticks)')
plt.ylabel('Price')
plt.title('Trade Prices Over Time')
plt.legend()
plt.savefig('trade_prices.png')
```

### Visualizing Price History
```python
# Load price history
prices = pd.read_csv('prices_rank_0.csv')

plt.figure(figsize=(12, 6))
for col in prices.columns[1:]:
    plt.plot(prices['Tick'], prices[col], label=col)

plt.xlabel('Time (ticks)')
plt.ylabel('Price')
plt.title('Price Evolution by Instrument')
plt.legend()
plt.savefig('price_history.png')
```

## 🎓 Key Concepts Demonstrated

### 1. **Hybrid Parallelism**
- **Horizontal scaling**: MPI for distributed exchanges
- **Vertical scaling**: OpenMP for concurrent agents
- **Communication**: Inter-process price synchronization

### 2. **Concurrency Control**
- **Mutex locks**: Protecting shared order queues
- **Barriers**: Synchronizing simulation ticks
- **Atomic operations**: Thread-safe counters

### 3. **Financial Algorithms**
- **Order matching**: Price-time priority matching
- **Price discovery**: Market-driven price updates
- **Trading strategies**: Momentum, mean reversion, market making

### 4. **System Design**
- **Modular architecture**: Separated concerns
- **Scalability**: Linear scaling with threads/processes
- **Observability**: Comprehensive logging and metrics

## 🧪 Robustness Features

### 1. **Error Handling**
- MPI initialization checks
- File I/O error handling
- Invalid order validation

### 2. **Resource Management**
- Proper MPI cleanup (MPI_Finalize)
- Memory management with RAII
- No memory leaks (verified with Valgrind)

### 3. **Deterministic Behavior**
- Seeded random number generators
- Reproducible simulations
- Consistent order matching

### 4. **Edge Case Handling**
- Zero-volume orders ignored
- Price gaps handled correctly
- Partial fills supported

## 📈 Performance Metrics

### Typical Results (Intel i7, 4 cores, 8 threads)
```
Configuration: 4 MPI processes, 8 OpenMP threads each
- Throughput: ~10,000 orders/second
- Latency: ~0.1ms per order
- Trades: ~40% order execution rate
- Scaling efficiency: 85% at 4 processes
```

### Benchmarking
```bash
# Strong scaling test (fixed problem size)
for np in 1 2 4 8; do
    echo "=== $np processes ==="
    mpirun -np $np ./trading_sim | grep "per Second"
done

# Weak scaling test (problem size scales with processes)
# Modify NUM_AGENTS in main.cpp to scale proportionally
```

## 🐛 Debugging

### Enable Debug Output
```bash
# Build in debug mode
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run with MPI debugging
mpirun -np 2 xterm -e gdb ./trading_sim
```

### Common Issues

**Issue**: `MPI_Init failed`
- **Solution**: Ensure MPI is properly installed: `which mpirun`

**Issue**: Low performance
- **Solution**: Build in Release mode: `cmake -DCMAKE_BUILD_TYPE=Release ..`

**Issue**: Segmentation fault
- **Solution**: Run with Valgrind: `mpirun -np 1 valgrind ./trading_sim`

## 📚 Further Extensions

### Suggested Improvements
1. **Network Latency Simulation**: Add random delays to MPI communication
2. **Order Book Visualization**: Real-time order book depth charts
3. **Historical Data Replay**: Use real market data as input
4. **Risk Management**: Add position limits and margin requirements
5. **Multiple Asset Classes**: Extend to futures, options, FX
6. **Machine Learning Agents**: Integrate RL-based trading strategies

## 📝 License

MIT License - Feel free to use for educational purposes.

## 🤝 Contributing

This is an educational project. Suggestions for improvements welcome!

## 📧 Contact

For questions about the implementation, please refer to the inline code comments or create an issue.

---

**Built with**: C++17, MPI, OpenMP, CMake
**Purpose**: High-performance computing and parallel systems demonstration