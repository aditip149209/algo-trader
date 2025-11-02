// ============================================================================
// include/exchange.h
// Exchange order book and matching engine
// Handles order submission, matching, and price discovery
// ============================================================================

#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <vector>
#include <queue>
#include <map>
#include <mutex>
#include <string>
#include <fstream>

// Order structure representing a buy or sell order
struct Order {
    int agent_id;           // ID of the agent placing the order
    int instrument_id;      // Which instrument to trade
    double price;           // Limit price
    int volume;             // Number of shares
    bool is_buy;            // true = buy, false = sell
    int timestamp;          // When order was placed
    int order_id;           // Unique order identifier
    
    Order() : agent_id(0), instrument_id(0), price(0.0), 
              volume(0), is_buy(true), timestamp(0), order_id(0) {}
};

// Trade structure representing an executed trade
struct Trade {
    int buy_agent_id;
    int sell_agent_id;
    int instrument_id;
    double price;
    int volume;
    int timestamp;
};

// Order book for a single instrument
class OrderBook {
private:
    // Priority queues for bids (buy orders) and asks (sell orders)
    // Bids sorted high to low, asks sorted low to high
    std::vector<Order> bids;
    std::vector<Order> asks;
    
    double last_price;
    std::vector<double> price_history;
    
public:
    OrderBook() : last_price(100.0) {
        price_history.push_back(last_price);
    }
    
    void add_order(const Order& order);
    std::vector<Trade> match_orders(int current_tick);
    double get_last_price() const { return last_price; }
    double get_historical_average() const;
    const std::vector<double>& get_price_history() const { return price_history; }
};

// Main exchange class managing multiple instruments
class Exchange {
private:
    int rank;                                    // MPI rank of this exchange
    int num_instruments;                         // Number of instruments traded
    std::vector<OrderBook> order_books;         // One order book per instrument
    std::queue<Order> pending_orders;           // Thread-safe order queue
    std::mutex order_mutex;                     // Protects pending orders
    std::vector<Trade> trade_log;               // All executed trades
    int next_order_id;                          // Counter for unique order IDs
    
public:
    Exchange(int rank, int num_instruments);
    
    // Thread-safe order submission
    void submit_order(const Order& order);
    
    // Process all pending orders and execute trades
    int process_orders(int current_tick);
    
    // Market data queries
    double get_price(int instrument_id) const;
    double get_historical_average(int instrument_id) const;
    std::vector<double> get_all_prices() const;
    
    // Update with global market information from other exchanges
    void update_global_prices(const std::vector<double>& global_prices, int local_rank);
    
    // Export results
    void export_trade_log(const std::string& filename) const;
    void export_price_history(const std::string& filename) const;
};

#endif // EXCHANGE_H

// ============================================================================
// include/agent.h
// Trading agent with multiple strategy implementations
// ============================================================================