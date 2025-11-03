#include "marketdata.h"

MarketDataManager::MarketDataManager(int rank_, int size_) : rank(rank_), size(size_) {}

std::vector<double> MarketDataManager::broadcast_prices(const std::vector<double> &local_prices)
{
    std::vector<double> summed(local_prices.size(), 0.0);
    // Sum across ranks
    MPI_Allreduce(local_prices.data(), summed.data(), (int)local_prices.size(), MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    // Average
    for (auto &v : summed)
        v /= size;
    return summed;
}

void MarketDataManager::synchronize()
{
    MPI_Barrier(MPI_COMM_WORLD);
}
