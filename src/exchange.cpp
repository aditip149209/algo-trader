#include "exchange.h"
#include <algorithm>
#include <numeric>
#include <sstream>

// ---------------- OrderBook -----------------

static bool bid_cmp(const Order &a, const Order &b)
{
    if (a.price != b.price)
        return a.price > b.price;     // higher first
    return a.timestamp < b.timestamp; // earlier first
}

static bool ask_cmp(const Order &a, const Order &b)
{
    if (a.price != b.price)
        return a.price < b.price;     // lower first
    return a.timestamp < b.timestamp; // earlier first
}

void OrderBook::add_order(const Order &order)
{
    if (order.is_buy)
    {
        bids.push_back(order);
    }
    else
    {
        asks.push_back(order);
    }
}

std::vector<Trade> OrderBook::match_orders(int current_tick)
{
    std::vector<Trade> trades;

    // sort order books
    std::sort(bids.begin(), bids.end(), bid_cmp);
    std::sort(asks.begin(), asks.end(), ask_cmp);

    size_t bi = 0, ai = 0;
    while (bi < bids.size() && ai < asks.size())
    {
        Order &bid = bids[bi];
        Order &ask = asks[ai];
        if (bid.price < ask.price)
            break; // no match

        int vol = std::min(bid.volume, ask.volume);
        Trade t{bid.agent_id, ask.agent_id, bid.instrument_id, (bid.price + ask.price) * 0.5, vol, current_tick};
        trades.push_back(t);

        last_price = t.price;
        price_history.push_back(last_price);

        bid.volume -= vol;
        ask.volume -= vol;
        if (bid.volume == 0)
            bi++;
        if (ask.volume == 0)
            ai++;
    }

    // erase filled orders
    bids.erase(bids.begin(), bids.begin() + bi);
    asks.erase(asks.begin(), asks.begin() + ai);

    return trades;
}

double OrderBook::get_historical_average() const
{
    if (price_history.empty())
        return last_price;
    double sum = std::accumulate(price_history.begin(), price_history.end(), 0.0);
    return sum / price_history.size();
}

// ---------------- Exchange -----------------

Exchange::Exchange(int rank_, int num_instruments_)
    : rank(rank_), num_instruments(num_instruments_), next_order_id(1)
{
    order_books.resize(num_instruments);
}

void Exchange::submit_order(const Order &order)
{
    std::lock_guard<std::mutex> lock(order_mutex);
    Order o = order;
    o.order_id = next_order_id++;
    pending_orders.push(o);
}

int Exchange::process_orders(int current_tick)
{
    // Move pending orders to corresponding order books
    {
        std::lock_guard<std::mutex> lock(order_mutex);
        while (!pending_orders.empty())
        {
            Order o = pending_orders.front();
            pending_orders.pop();
            if (o.instrument_id >= 0 && o.instrument_id < (int)order_books.size())
            {
                order_books[o.instrument_id].add_order(o);
            }
        }
    }

    int trades_total = 0;
    for (size_t i = 0; i < order_books.size(); ++i)
    {
        auto t = order_books[i].match_orders(current_tick);
        trades_total += (int)t.size();
        trade_log.insert(trade_log.end(), t.begin(), t.end());
    }
    return trades_total;
}

double Exchange::get_price(int instrument_id) const
{
    if (instrument_id >= 0 && instrument_id < (int)order_books.size())
    {
        return order_books[instrument_id].get_last_price();
    }
    return 0.0;
}

double Exchange::get_historical_average(int instrument_id) const
{
    if (instrument_id >= 0 && instrument_id < (int)order_books.size())
    {
        return order_books[instrument_id].get_historical_average();
    }
    return 0.0;
}

std::vector<double> Exchange::get_all_prices() const
{
    std::vector<double> prices(order_books.size());
    for (size_t i = 0; i < order_books.size(); ++i)
    {
        prices[i] = order_books[i].get_last_price();
    }
    return prices;
}

void Exchange::update_global_prices(const std::vector<double> &global_prices, int /*local_rank*/)
{
    // In this simplified implementation we do not override local prices.
    // You could incorporate global averages here if desired.
    (void)global_prices;
}

void Exchange::export_trade_log(const std::string &filename) const
{
    std::ofstream ofs(filename);
    ofs << "Timestamp,Instrument,Price,Volume,BuyAgent,SellAgent\n";
    for (const auto &t : trade_log)
    {
        ofs << t.timestamp << "," << t.instrument_id << "," << t.price << "," << t.volume
            << "," << t.buy_agent_id << "," << t.sell_agent_id << "\n";
    }
}

void Exchange::export_price_history(const std::string &filename) const
{
    std::ofstream ofs(filename);
    ofs << "Tick";
    for (size_t i = 0; i < order_books.size(); ++i)
        ofs << ",Instrument_" << i;
    ofs << "\n";

    // Find longest history length
    size_t max_len = 0;
    for (const auto &ob : order_books)
        max_len = std::max(max_len, ob.get_price_history().size());

    for (size_t tick = 0; tick < max_len; ++tick)
    {
        ofs << tick;
        for (const auto &ob : order_books)
        {
            const auto &hist = ob.get_price_history();
            double v = tick < hist.size() ? hist[tick] : (hist.empty() ? 0.0 : hist.back());
            ofs << "," << v;
        }
        ofs << "\n";
    }
}
