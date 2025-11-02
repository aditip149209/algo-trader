// ============================================================================
// include/marketdata.h
// Market data manager for cross-exchange communication via MPI
// ============================================================================

#ifndef MARKETDATA_H
#define MARKETDATA_H

#include <mpi.h>
#include <vector>

// Manages market data synchronization across MPI ranks
class MarketDataManager {
private:
    int rank;           // This process's rank
    int size;           // Total number of processes
    
public:
    MarketDataManager(int rank, int size);
    
    // Broadcast local prices to all other exchanges and receive theirs
    // Returns aggregated price information from all exchanges
    std::vector<double> broadcast_prices(const std::vector<double>& local_prices);
    
    // Synchronize all exchanges at a barrier point
    void synchronize();
};

#endif // MARKETDATA_H
