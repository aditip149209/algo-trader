// Minimal MPI test harness to validate build on Windows
#include <mpi.h>
#include <iostream>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank = 0, size = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (rank == 0)
    {
        std::cout << "=== Trading Simulator Test Suite (smoke) ===\n";
        std::cout << "MPI ranks: " << size << "\n";
        std::cout << "[PASS] mpi_init\n";
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}
