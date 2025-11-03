#include "agent.h"
#include <algorithm>

Agent::Agent(int thread_id_, int agent_id_, AgentStrategy strategy_)
    : thread_id(thread_id_), agent_id(agent_id_), strategy(strategy_), rng(static_cast<unsigned int>(agent_id_)), momentum_threshold(0.5), reversion_threshold(0.5), position(0) {}

std::vector<Order> Agent::generate_orders(
    int instrument_id,
    double current_price,
    double historical_average,
    int timestamp)
{

    switch (strategy)
    {
    case AgentStrategy::RANDOM_WALK:
        return random_walk_strategy(instrument_id, current_price, timestamp);
    case AgentStrategy::MOMENTUM:
        return momentum_strategy(instrument_id, current_price, historical_average, timestamp);
    case AgentStrategy::MEAN_REVERSION:
        return mean_reversion_strategy(instrument_id, current_price, historical_average, timestamp);
    case AgentStrategy::MARKET_MAKER:
    default:
        return market_maker_strategy(instrument_id, current_price, timestamp);
    }
}

std::vector<Order> Agent::random_walk_strategy(int instrument_id, double current_price, int timestamp)
{
    std::uniform_real_distribution<double> uni(0.0, 1.0);
    std::uniform_int_distribution<int> vol(1, 10);
    std::vector<Order> orders;

    bool buy = uni(rng) < 0.5;
    double px = current_price * (buy ? 0.99 : 1.01);
    Order o;
    o.agent_id = agent_id;
    o.instrument_id = instrument_id;
    o.price = px;
    o.volume = vol(rng);
    o.is_buy = buy;
    o.timestamp = timestamp;
    o.order_id = 0;
    orders.push_back(o);
    return orders;
}

std::vector<Order> Agent::momentum_strategy(int instrument_id, double current_price, double historical_average, int timestamp)
{
    std::uniform_int_distribution<int> vol(1, 10);
    std::vector<Order> orders;

    bool buy = current_price > historical_average * (1.0 + 0.001 * momentum_threshold);
    double px = current_price * (buy ? 1.005 : 0.995);
    Order o;
    o.agent_id = agent_id;
    o.instrument_id = instrument_id;
    o.price = px;
    o.volume = vol(rng);
    o.is_buy = buy;
    o.timestamp = timestamp;
    o.order_id = 0;
    orders.push_back(o);
    return orders;
}

std::vector<Order> Agent::mean_reversion_strategy(int instrument_id, double current_price, double historical_average, int timestamp)
{
    std::uniform_int_distribution<int> vol(1, 10);
    std::vector<Order> orders;

    bool buy = current_price < historical_average * (1.0 - 0.001 * reversion_threshold);
    double px = current_price * (buy ? 1.002 : 0.998);
    Order o;
    o.agent_id = agent_id;
    o.instrument_id = instrument_id;
    o.price = px;
    o.volume = vol(rng);
    o.is_buy = buy;
    o.timestamp = timestamp;
    o.order_id = 0;
    orders.push_back(o);
    return orders;
}

std::vector<Order> Agent::market_maker_strategy(int instrument_id, double current_price, int timestamp)
{
    std::uniform_int_distribution<int> vol(1, 5);
    std::vector<Order> orders;

    // Place a bid and an ask around the mid price
    {
        Order bid;
        bid.agent_id = agent_id;
        bid.instrument_id = instrument_id;
        bid.price = current_price * 0.999;
        bid.volume = vol(rng);
        bid.is_buy = true;
        bid.timestamp = timestamp;
        bid.order_id = 0;
        orders.push_back(bid);
    }
    {
        Order ask;
        ask.agent_id = agent_id;
        ask.instrument_id = instrument_id;
        ask.price = current_price * 1.001;
        ask.volume = vol(rng);
        ask.is_buy = false;
        ask.timestamp = timestamp;
        ask.order_id = 0;
        orders.push_back(ask);
    }
    return orders;
}
