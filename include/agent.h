// ============================================================================
// include/agent.h
// Trading agent with multiple strategy implementations
// ============================================================================

#ifndef AGENT_H
#define AGENT_H

#include "exchange.h"
#include <random>
#include <vector>

// Different trading strategies
enum class AgentStrategy {
    RANDOM_WALK,      // Random buy/sell decisions
    MOMENTUM,         // Follow price trends
    MEAN_REVERSION,   // Buy low, sell high
    MARKET_MAKER      // Place orders on both sides
};

// Trading agent that generates orders based on strategy
class Agent {
private:
    int thread_id;                  // OpenMP thread ID
    int agent_id;                   // Unique agent ID
    AgentStrategy strategy;         // Trading strategy
    std::mt19937 rng;              // Random number generator
    
    // Strategy parameters
    double momentum_threshold;
    double reversion_threshold;
    int position;                   // Current position in instrument
    
public:
    Agent(int thread_id, int agent_id, AgentStrategy strategy);
    
    // Generate orders based on current market conditions
    std::vector<Order> generate_orders(
        int instrument_id,
        double current_price,
        double historical_average,
        int timestamp
    );
    
private:
    // Strategy-specific order generation
    std::vector<Order> random_walk_strategy(
        int instrument_id, double current_price, int timestamp);
    
    std::vector<Order> momentum_strategy(
        int instrument_id, double current_price, 
        double historical_average, int timestamp);
    
    std::vector<Order> mean_reversion_strategy(
        int instrument_id, double current_price,
        double historical_average, int timestamp);
    
    std::vector<Order> market_maker_strategy(
        int instrument_id, double current_price, int timestamp);
};

#endif // AGENT_H

// ============================================================================
// include/marketdata.h
// Market data manager for cross-exchange communication via MPI
// ============================================================================