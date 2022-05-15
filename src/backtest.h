//
// Created by Abhishek Aggarwal on 09/12/2021.
//

#ifndef CRYPTONITE_BACKTEST_H
#define CRYPTONITE_BACKTEST_H


#include "strategy.h"
#include "dataset.h"
#include "datastore.h"
#include "../include/json.h"

using json = nlohmann::json;

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


struct Equity {
    double quoteEquity;
    double baseEquity;
    double totalInQuote;
    double borrowAllowance;

    Equity(double quoteEquity=0.0, double baseEquity=0.0, double totalInQuote=0.0, double borrowAllowance=0.0) {
        this->quoteEquity = quoteEquity;
        this->baseEquity = baseEquity;
        this->totalInQuote = totalInQuote;
        this->borrowAllowance = borrowAllowance;
    }

    Equity getUpdatedEquity(double currentPrice, double quoteDelta = 0.0, double baseDelta = 0.0, double borrowAllowanceDelta = 0.0) const;
    json toJson() const;
};

struct Order {
    int bar;
    double quoteSize;
    double baseSize;
    double price;
    bool isLong;

    json toJson(){
        json j;
        j["bar"] = bar;
        j["quoteSize"] = quoteSize;
        j["baseSize"] = baseSize;
        j["price"] = price;
        j["isLong"] = isLong;
        return j;
    }
};

struct Trade {
    Order openingOrder{};
    Order closingOrder{};
    Equity equityAtOpen{};
    Equity equityAtClose{};
    double takeProfit{INFINITY};
    double stopLoss{INFINITY};
    bool trailingSl{false};
    bool active{false};
    bool is_long(){
        return openingOrder.isLong;
    }

    static Trade openTrade(const Equity& equityAtOpen, const Order& openingOrder, double takeProfit, double stopLoss, bool trailingSl){
        Trade trade;
        trade.active = true;
        trade.equityAtOpen = equityAtOpen;
        trade.openingOrder = openingOrder;
        trade.takeProfit = takeProfit;
        trade.stopLoss = stopLoss;
        trade.trailingSl = trailingSl;
        return trade;
    }

    void closeTrade(const Equity& equityAtClose, const Order& closingOrder){
        active = false;
        this->equityAtClose = equityAtClose;
        this->closingOrder = closingOrder;
    }

    json toJson(){
        json j;
        j["openingOrder"] = openingOrder.toJson();
        j["closingOrder"] = closingOrder.toJson();
        j["equityAtOpen"] = equityAtOpen.toJson();
        j["equityAtClose"] = equityAtClose.toJson();
        j["takeProfit"] = takeProfit;
        j["stopLoss"] = stopLoss;
        j["trailingSl"] = trailingSl;
        j["active"] = active;
        return j;
    }
};


struct Metrics {
    double CAGR{0};
    double CAGROverAvgDrawDown{0};
    double CAGROverMaxDrawDown{0};
    double totalReturn{0};
    double winRate{0.0};
    double avgWin{0.0};
    double avgLoss{0.0};
    double loseRate{0.0};
    double profitFactor{0};
    double profitFactorBetter{0};  // This accounts for the winRate so a single huge winning rate cannot bias the results
    double maxDrawDown{0};
    double avgDrawDown{0};
    int numTrades{0};

    bool operator>(const Metrics& other) const {
        return this->metric() > other.metric();
    }

    double metric() const{
        return numTrades < 100 ? 0.0 : numTrades * CAGROverAvgDrawDown;
    }

    json toJson() const {
        json j;
        j["numTrades"] = numTrades;
        j["profitFactor"] = profitFactor;
        j["profitFactorBetter"] = profitFactorBetter;
        j["winRate"] = winRate;
        j["loseRate"] = loseRate;
        j["avgWinReturn"] = avgWin;
        j["avgLossReturn"] = avgLoss;
        j["CAGROverAvgDrawDown"] = CAGROverAvgDrawDown;
        j["CAGROverMaxDrawDown"] = CAGROverMaxDrawDown;
        j["metric"] = this->metric();
        j["CAGR"] = CAGR;
        j["totalReturn"] = totalReturn;
        j["avgDrawDown"] = avgDrawDown;
        j["maxDrawDown"] = maxDrawDown;
        return j;
    }

};


struct Backtest {
    std::vector<Equity> equityCurve{};
    std::vector<Trade> trades{};
    Metrics metrics{};
    Strategy strategy;
    DataSetConfig dataSetConfig;

    Backtest(const Strategy& strategy, const DataSetConfig& dataSetConfig): strategy(strategy), dataSetConfig(dataSetConfig){};


    bool hasActiveTrade(){
        return not trades.empty() && trades.back().active;
    }

    Trade& getActiveTrade(){
        return trades.back();
    }

    void openTrade(const Equity& equityAtOpen, const Order& openingOrder, double takeProfit, double stopLoss, bool trailingSl){
        trades.emplace_back(Trade::openTrade(equityAtOpen, openingOrder, takeProfit, stopLoss, trailingSl));
    }

    void closeTrade(const Equity& equityAtClose, const Order& closingOrder){
        getActiveTrade().closeTrade(equityAtClose, closingOrder);
    }

    Equity enterLong(int bar, double lastPrice, const Equity& equity, const Strategy& strategy);
    Equity enterShort(int bar, double lastPrice, const Equity& equity, const Strategy& strategy);
    Equity exitLong(int bar, double lastPrice, const Equity& equity, const Strategy& strategy);
    Equity exitShort(int bar, double lastPrice, const Equity& equity, const Strategy& strategy);

    bool operator>(const Backtest& other) const {
        return metrics > other.metrics;
    }

    void computeMetrics(const Dataset& dataset){
        if(equityCurve.size() == 0) return;
        metrics.numTrades = trades.size();
        metrics.totalReturn = equityCurve.back().totalInQuote / equityCurve[0].totalInQuote - 1.0;
        std::shared_ptr<double[]> cumEquityScale(new double [metrics.numTrades]);
        std::shared_ptr<double[]> drawDown(new double [metrics.numTrades]);

        double numWins = 0, numLoses = 0;
        double sumProfits{0.0}, sumLosses{0.0};
        double cumScale = 1.0, maxScale = 0.0, sumDrawDown = 0.0, maxDrawDown = 0.0;
        double numDD = 0.0;
        for(int i=0; i<metrics.numTrades; i++){
            double s = trades[i].equityAtClose.totalInQuote/trades[i].equityAtOpen.totalInQuote;
            double r = s - 1.0;
            if (r > 0){
                numWins += 1;
                sumProfits += r;
            } else {
                numLoses += 1;
                sumLosses += -r;
            }
            cumScale *= s;
            maxScale = std::max(maxScale, cumScale);
            cumEquityScale[i] = cumScale;
            drawDown[i] = 1.0 - (cumScale/maxScale);
            if(drawDown[i] > 0){
                numDD += 1.0;
                maxDrawDown = std::max(maxDrawDown, drawDown[i]);
                sumDrawDown += drawDown[i];
            }
        }

        double totalReturn = cumScale - 1.0;
        metrics.avgDrawDown = sumDrawDown / numDD;  // TODO(abhi) This is Wrong, only divide by drawdowns that actually happened
        metrics.maxDrawDown = maxDrawDown;
        metrics.winRate = numWins / metrics.numTrades;
        metrics.loseRate = numLoses / metrics.numTrades;

        metrics.avgWin = sumProfits / numWins;
        metrics.avgLoss = sumLosses / numLoses;

        metrics.profitFactor = sumProfits / sumLosses;
        metrics.profitFactorBetter = (metrics.winRate * metrics.avgWin) / (metrics.loseRate * metrics.avgLoss);
        std::string msg =   "" + std::to_string(totalReturn) + " != " + std::to_string(metrics.totalReturn);
        double divergence = totalReturn - metrics.totalReturn;
        if(divergence > 0.00001 or divergence < -0.00001){
            std::cout << msg <<std::endl;
        }
        assert (divergence < 0.00001 and divergence > -0.00001);
        double numDays = dataset.durationDays();
        metrics.CAGR = std::pow((equityCurve.back().totalInQuote/equityCurve.front().totalInQuote), (365.0/numDays))  - 1.0;
        metrics.CAGROverAvgDrawDown = metrics.CAGR / metrics.avgDrawDown;
        metrics.CAGROverMaxDrawDown = metrics.CAGR /  metrics.maxDrawDown;
    }

    json toJson() const {
        json j;
        j["metrics"] = metrics.toJson();
        j["dataset"] = dataSetConfig.toJson();
        j["strategy"] = strategy.toJson();
        return j;
    }

};

struct Backtester {
    AcceptanceConfig acceptanceConfig;
    DataSetConfig datasetConfig;
    Dataset dataset;

    Backtester(AcceptanceConfig& acceptanceConfig, DataSetConfig& datasetConfig, std::string datastorePath):
            acceptanceConfig(acceptanceConfig),
            dataset(DataStore(datasetConfig, datastorePath).getDataset())
    {
        Indicator::setup(dataset);

    };

    Backtest evaluate(const Strategy &strategy);
};

Signal computeSignal(Strategy &strategy, const Dataset &dataset);

#endif //CRYPTONITE_BACKTEST_H
