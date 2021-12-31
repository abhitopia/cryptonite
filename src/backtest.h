//
// Created by Abhishek Aggarwal on 09/12/2021.
//

#ifndef CRYPTONITE_BACKTEST_H
#define CRYPTONITE_BACKTEST_H


#include "strategy.h"
#include "metrics.h"
#include "dataset.h"
#include "datastore.h"

struct Signal {
    std::shared_ptr<bool[]> shouldLongEnter;
    std::shared_ptr<bool[]> shouldShortEnter;
    std::shared_ptr<bool[]> shouldLongExit;
    std::shared_ptr<bool[]> shouldShortExit;

    int max_possible_entries(int numBars){
        int maxEntries = 0;
        for(int i=0; i <  numBars; i++){
            if(shouldShortEnter[i] || shouldLongEnter[i]){
                maxEntries += 1;
            }
        }
        return maxEntries;
    }
};

struct Order {
    int bar;
    double quoteSize;
    double baseSize;
    double price;
    bool isLong;
};

struct Trade {
    Order openingOrder{};
    Order closingOrder{};
    double takeProfit{INFINITY};
    double stopLoss{INFINITY};
    bool trailingSl{false};
    bool active{false};
    bool is_long(){
        return openingOrder.isLong;
    }

    static Trade openTrade(const Order& openingOrder, double takeProfit, double stopLoss, bool trailingSl){
        Trade trade;
        trade.active = true;
        trade.openingOrder = openingOrder;
        trade.takeProfit = takeProfit;
        trade.stopLoss = stopLoss;
        trade.trailingSl = trailingSl;
        return trade;
    }

    void closeTrade(const Order& closingOrder){
        active = false;
        this->closingOrder = closingOrder;
    }
};


struct StoppingCriteria {
    int minNumTrades{10};
    double minTotalEquityFraction{0.5};
    StoppingCriteria(int minNumTrades=200, double minTotalEquityFraction=0.5){
        assert(minNumTrades > 100 && "Minimum number of trades less than 100 is not statistically significant");
        assert(minTotalEquityFraction < 1.0 && minTotalEquityFraction > 0);
        this->minNumTrades = minNumTrades;
        this->minTotalEquityFraction = minTotalEquityFraction;
    }
};

struct Backtest {
    std::vector<Equity> equityCurve{};
    std::vector<Trade> trades{};
    Metrics metrics{};

    bool hasActiveTrade(){
        return not trades.empty() && trades.back().active;
    }

    Trade& getActiveTrade(){
        return trades.back();
    }

    void openTrade(const Order& openingOrder, double takeProfit, double stopLoss, bool trailingSl){
        trades.emplace_back(Trade::openTrade(openingOrder, takeProfit, stopLoss, trailingSl));
    }

    void closeTrade(const Order& closingOrder){
        getActiveTrade().closeTrade(closingOrder);
    }

    Equity enterLong(int bar, double lastPrice, const Equity& equity, const Strategy& strategy);
    Equity enterShort(int bar, double lastPrice, const Equity& equity, const Strategy& strategy);
    Equity exitLong(int bar, double lastPrice, const Equity& equity, const Strategy& strategy);
    Equity exitShort(int bar, double lastPrice, const Equity& equity, const Strategy& strategy);
};

struct Backtester {
    StrategyGenConfig config;
    Dataset dataset;

    Backtester(StrategyGenConfig& config, std::string datastorePath):
            config(config),
            dataset(DataStore(config.dataSetConfig, datastorePath).getDataset())
    {
        Indicator::setup(dataset);

    };

    Backtest evaluate(const Strategy &strategy);

};

Signal computeSignal(Strategy &strategy, const Dataset &dataset);

#endif //CRYPTONITE_BACKTEST_H
