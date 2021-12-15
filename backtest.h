//
// Created by Abhishek Aggarwal on 09/12/2021.
//

#ifndef CRYPTONITE_BACKTEST_H
#define CRYPTONITE_BACKTEST_H


#include "strategy.h"

struct Signal {
    shared_ptr<bool[]> shouldLongEnter;
    shared_ptr<bool[]> shouldShortEnter;
    shared_ptr<bool[]> shouldLongExit;
    shared_ptr<bool[]> shouldShortExit;
};

struct Order {
    int bar;
    double quoteSize;
    double baseSize;
    double price;
    bool isLong;
};

struct Trade {
    Order openingOrder;
    Order closingOrder;
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


struct Equity {
    double quoteEquity{0.0};
    double baseEquity{0.0};
    double totalInQuote{0.0};
    double borrowAllowance{0.0};

    Equity(double quoteEquity, double baseEquity, double totalInQuote, double borrowAllowance) {
        this->quoteEquity = quoteEquity;
        this->baseEquity = baseEquity;
        this->totalInQuote = totalInQuote;
        this->borrowAllowance = borrowAllowance;

    }

    Equity getUpdatedEquity(double currentPrice, double quoteDelta = 0.0, double baseDelta = 0.0, double borrowAllowanceDelta = 0.0) const {
        Equity newEquity{*this};
        newEquity.quoteEquity = quoteEquity + quoteDelta;
        newEquity.baseEquity = baseEquity + baseDelta;
        newEquity.borrowAllowance = borrowAllowance + borrowAllowanceDelta;
        newEquity.totalInQuote = newEquity.quoteEquity + newEquity.baseEquity * currentPrice;
        return newEquity;
    }
};

struct Backtest {
    vector<Equity> equityCurve{};
    vector<Trade> trades{};

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

    void operator()(const Strategy &strategy, const Dataset &dataset);
};

Signal computeSignal(Strategy &strategy, const Dataset &dataset);

#endif //CRYPTONITE_BACKTEST_H
