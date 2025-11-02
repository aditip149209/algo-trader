# Quick Start Guide

Get the trading simulator running in 5 minutes!

## Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libopenmpi-dev openmpi-bin
```

**macOS:**
```bash
brew install cmake open-mpi
```

## Build and Run

### 1. Clone/Navigate to Project
```bash
cd trading-sim/
```

### 2. Build
```bash
chmod +x scripts/*.sh
./scripts/build.sh
```

### 3. Run Tests
```bash
./scripts/build.sh --test
```

### 4. Run Simulation
```bash
# Quick run with 4 exchanges
mpirun -np 4 ./build/trading_sim

# Or use the runner script
./scripts/run_simulation.sh -np 4 -nt 8
```

### 5. Validate Everything
```bash
./scripts/validate_correctness.sh
```

## Expected Output

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

## Output Files

After running, you'll get:
- `trades_rank_X.csv` - All executed trades
- `prices_rank_X.csv` - Price history for each instrument

## Visualize Results

If you have Python installed:
```bash
pip3 install matplotlib pandas
python3 scripts/analyze_results.py .
```

This generates:
- `trade_prices.png` - Price chart
- `price_history.png` - Price evolution
- `trade_volume.png` - Volume distribution
- `price_volatility.png` - Volatility analysis
- `statistics.txt` - Statistical summary

## Common Issues

**Issue**: `mpirun: command not found`
```bash
sudo apt-get install openmpi-bin
```

**Issue**: Low performance
```bash
# Rebuild in Release mode
./scripts/build.sh --clean
```

**Issue**: Tests fail
```bash
# Check your MPI installation
mpirun --version
```

## Next Steps

1. **Modify parameters** in `src/main.cpp`:
   - `NUM_INSTRUMENTS` - More/fewer instruments
   - `NUM_AGENTS` - More/fewer traders
   - `SIMULATION_TICKS` - Longer/shorter simulation

2. **Add new strategies** in `src/agent.cpp`:
   - Implement custom trading logic
   - Test different market behaviors

3. **Run performance tests**:
   ```bash
   ./scripts/run_simulation.sh --scaling-test
   ```

4. **Explore the code**:
   - Start with `src/main.cpp` for the flow
   - Check `include/exchange.h` for data structures
   - Review `src/agent.cpp` for strategy examples

## Architecture Overview

```
┌─────────────────────────────────────┐
│         MPI Layer (Distributed)     │
│  ┌────────┐  ┌────────┐  ┌────────┐│
│  │Exchange│  │Exchange│  │Exchange││
│  │Rank 0  │  │Rank 1  │  │Rank 2  ││
│  └────────┘  └────────┘  └────────┘│
│       ↕           ↕           ↕     │
│   Price Sync  Price Sync  Price Sync│
└─────────────────────────────────────┘
         ↓           ↓           ↓
┌─────────────────────────────────────┐
│      OpenMP Layer (Shared Memory)   │
│  ┌────┐ ┌────┐ ┌────┐ ┌────┐       │
│  │Agnt│ │Agnt│ │Agnt│ │Agnt│ ...   │
│  │ 0  │ │ 1  │ │ 2  │ │ 3  │       │
│  └────┘ └────┘ └────┘ └────┘       │
│     ↓      ↓      ↓      ↓          │
│        Order Submission              │
└─────────────────────────────────────┘
```

## Key Concepts

- **MPI Processes**: Independent exchanges that communicate
- **OpenMP Threads**: Concurrent traders within each exchange
- **Order Matching**: Price-time priority algorithm
- **Strategies**: Random, momentum, mean-reversion, market-making

## Need Help?

Check the full documentation: `README.md`

Run validation: `./scripts/validate_correctness.sh`

---

**You're all set! Happy trading! 📈**