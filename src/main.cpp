#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <memory>
#include <chrono>
#include <iomanip>
#include <vector>
#include <string>
#include "exchange.h"
#include "agent.h"
#include "marketdata.h"
#include "utils.h"

using namespace std;

int main(int argc, char **argv)
{
    // Initialize MPI environment
    // Each MPI rank represents a separate exchange/market node
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get this process's rank
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Get total number of processes

    // Simulation parameters
    const int NUM_INSTRUMENTS = 3;     // Number of trading instruments per exchange
    const int NUM_AGENTS = 8;          // Number of trader agents (OpenMP threads)
    const int SIMULATION_TICKS = 1000; // Total simulation time steps
    const int AGENTS_PER_INSTRUMENT = NUM_AGENTS / NUM_INSTRUMENTS;

    // Set OpenMP thread count
    omp_set_num_threads(NUM_AGENTS);

    if (rank == 0)
    {
        std::cout << "=== Algorithmic Trading Simulator ===" << std::endl;
        std::cout << "MPI Processes (Exchanges): " << size << std::endl;
        std::cout << "OpenMP Threads (Agents) per Process: " << NUM_AGENTS << std::endl;
        std::cout << "Instruments per Exchange: " << NUM_INSTRUMENTS << std::endl;
        std::cout << "Simulation Ticks: " << SIMULATION_TICKS << std::endl;
        std::cout << "======================================" << std::endl;
    }

    // Create exchange for this rank
    Exchange exchange(rank, NUM_INSTRUMENTS);

    // Initialize market data manager for cross-exchange communication
    MarketDataManager md_manager(rank, size);

    // Performance tracking
    auto start_time = std::chrono::high_resolution_clock::now();

    // Statistics tracking
    long long total_orders = 0;
    long long total_trades = 0;

    // Main simulation loop
    for (int tick = 0; tick < SIMULATION_TICKS; ++tick)
    {

// Phase 1: Agents generate and submit orders (parallel)
#pragma omp parallel reduction(+ : total_orders)
        {
            int thread_id = omp_get_thread_num();

            // Each agent is assigned to trade specific instruments
            int instrument_id = thread_id % NUM_INSTRUMENTS;

            // Create agent with strategy based on thread ID
            AgentStrategy strategy = static_cast<AgentStrategy>(thread_id % 4);
            Agent agent(thread_id, rank * NUM_AGENTS + thread_id, strategy);

            // Agent observes market and decides on action
            double current_price = exchange.get_price(instrument_id);
            double historical_avg = exchange.get_historical_average(instrument_id);

            // Generate orders based on strategy
            std::vector<Order> orders = agent.generate_orders(
                instrument_id,
                current_price,
                historical_avg,
                tick);

            // Submit orders to exchange (thread-safe)
            for (const auto &order : orders)
            {
                exchange.submit_order(order);
                total_orders++;
            }
        }

        // Phase 2: Exchange processes orders and matches trades (sequential)
        int trades_this_tick = exchange.process_orders(tick);
        total_trades += trades_this_tick;

        // Phase 3: Broadcast price updates across exchanges (MPI communication)
        vector<double> local_prices = exchange.get_all_prices();
        vector<double> global_prices = md_manager.broadcast_prices(local_prices);

        // Update local exchange with global market information
        exchange.update_global_prices(global_prices, rank);

        // Phase 4: Synchronize all exchanges at end of tick
        MPI_Barrier(MPI_COMM_WORLD);

        // Progress reporting (rank 0 only, every 100 ticks)
        if (rank == 0 && (tick + 1) % 100 == 0)
        {
            cout << "Tick " << std::setw(4) << (tick + 1)
                 << " | Orders: " << setw(6) << total_orders
                 << " | Trades: " << setw(6) << total_trades
                 << std::endl;
        }
    }

    // Calculate performance metrics
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(
                        end_time - start_time)
                        .count();

    // Gather statistics from all ranks
    long long global_orders = 0;
    long long global_trades = 0;
    MPI_Reduce(&total_orders, &global_orders, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&total_trades, &global_trades, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    // Export results to file (each rank writes its own file)
    exchange.export_trade_log("trades_rank_" + to_string(rank) + ".csv");
    exchange.export_price_history("prices_rank_" + to_string(rank) + ".csv");

    // Final report (rank 0 only)
    if (rank == 0)
    {
        cout << "\n=== Simulation Complete ===" << endl;
        cout << "Total Execution Time: " << duration << " ms" << endl;
        cout << "Global Orders Submitted: " << global_orders << endl;
        cout << "Global Trades Executed: " << global_trades << endl;
        cout << "Orders per Second: "
             << (global_orders * 1000.0 / duration) << endl;
        cout << "Trades per Second: "
             << (global_trades * 1000.0 / duration) << endl;
        cout << "==========================" << std::endl;
    }

    // Cleanup MPI environment
    MPI_Finalize();
    return 0;
}