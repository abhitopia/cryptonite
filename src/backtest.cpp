//
// Created by Abhishek Aggarwal on 09/12/2021.
//

#include "backtest.h"

Equity Equity::getUpdatedEquity(double currentPrice, double quoteDelta, double baseDelta, double borrowAllowanceDelta) const {
    Equity newEquity{};  // Not using {*this} will not call a copy constructor
    newEquity.quoteEquity = quoteEquity + quoteDelta;
    newEquity.baseEquity = baseEquity + baseDelta;
    newEquity.borrowAllowance = borrowAllowance + borrowAllowanceDelta;
    newEquity.totalInQuote = newEquity.quoteEquity + newEquity.baseEquity * currentPrice;
    return newEquity;
}

json Equity::toJson() const {
    json j;
    j["quoteEquity"] = quoteEquity;
    j["baseEquity"] = baseEquity;
    j["borrowAllowance"] = borrowAllowance;
    j["totalInQuote"] = totalInQuote;
    return j;
}

Signal computeSignal(const Strategy &strategy, const Dataset &dataset) {
    Signal signal;
//    signal.shouldLongEnter = strategy.entryCriteria.apply(dataset, false);
//    signal.shouldLongExit = strategy.exitCriteria.apply(dataset, false);
//    if(strategy.positionOpenConfig.bidirectional){
//        signal.shouldShortEnter = strategy.entryCriteria.apply(dataset, true);
//        signal.shouldShortExit = strategy.exitCriteria.apply(dataset, true);
//    }



    #pragma omp parallel sections default(none) shared(signal, dataset, strategy) if(MULTITHREADED)
    {
        #pragma omp section
        {
            signal.shouldLongEnter = strategy.entryCriteria.apply(dataset, false);
        }

        #pragma omp  section
        {
            signal.shouldLongExit = strategy.exitCriteria.apply(dataset, false);
        }

        #pragma omp  section
        {
            if(strategy.positionOpenConfig.bidirectional){
                signal.shouldShortEnter = strategy.entryCriteria.apply(dataset, true);
            }
        }

        #pragma omp  section
        {
            if(strategy.positionOpenConfig.bidirectional){
                signal.shouldShortExit = strategy.exitCriteria.apply(dataset, true);
            }
        }
    }

    return signal;
}


Equity Backtest::enterLong(int bar, double lastPrice, const Equity& equity, const Strategy& strategy){

    const PositionOpenConfig& openConfig = strategy.positionOpenConfig;
    const PositionCloseConfig& closeConfig = strategy.positionCloseConfig;
    const BrokerConfig& brokerConfig = strategy.brokerConfig;

    // In case, we were not able to return the borrowed base, availableQuoteEquity < 0.0
    double availableQuoteEquity = std::max(0.0, equity.quoteEquity);
    double tradeSize = openConfig.isAbsolute ? openConfig.quoteSize : (openConfig.quoteSize * availableQuoteEquity);
    double quoteLost = std::min(availableQuoteEquity, tradeSize);

    assert(equity.baseEquity == 0.0 && "Base Equity must be zero before opening a new trade");

    if(quoteLost == 0){
        return equity;
    }

    // Price increased due to slippage
    double currentPrice = lastPrice * (1.0 + brokerConfig.slippage);
    double baseGained = quoteLost / currentPrice;

    // Commission is always applied on purchased asset
    baseGained = baseGained * (1.0 - brokerConfig.commission);

    Equity newEquity = equity.getUpdatedEquity(currentPrice, -quoteLost, baseGained, 0.0);

    Order order{};
    order.bar = bar;
    order.quoteSize = -quoteLost;
    order.baseSize = baseGained;
    order.price = currentPrice;
    order.isLong = true;

    double takeProfit = lastPrice * (1.0 + closeConfig.takeProfit);
    double stopLoss = lastPrice * (1.0 - closeConfig.stopLoss);
    openTrade(equity, order, takeProfit, stopLoss, closeConfig.trailingSl);

//    std::cout << "Opening Long Trade: " << std::setw(4) << getActiveTrade().toJson() << std::endl;
    return newEquity;
}

Equity Backtest::enterShort(int bar, double lastPrice, const Equity& equity, const Strategy& strategy){

    const PositionOpenConfig& openConfig = strategy.positionOpenConfig;
    const PositionCloseConfig& closeConfig = strategy.positionCloseConfig;
    const BrokerConfig& brokerConfig = strategy.brokerConfig;
    const DepositConfig& depositConfig = strategy.depositConfig;

    double availableQuoteEquity = std::max(0.0, equity.quoteEquity);
    double tradeSize = openConfig.isAbsolute ? openConfig.quoteSize : (openConfig.quoteSize * availableQuoteEquity);
    double quoteForBorrow = std::min(availableQuoteEquity, tradeSize);

    // Price increased due to slippage (for borrow, less borrowing)
    double currentPrice = lastPrice * (1.0 + brokerConfig.slippage);
    double baseBorrowed = quoteForBorrow / currentPrice;

    baseBorrowed = std::min(baseBorrowed, equity.borrowAllowance);

    if(baseBorrowed == 0){
        return equity;
    }

    // Commission is always applied on purchased asset
    double quoteGained = baseBorrowed * currentPrice * (1.0 - brokerConfig.commission);

    Equity newEquity = equity.getUpdatedEquity(currentPrice, quoteGained, -baseBorrowed, -baseBorrowed);

    Order order{};
    order.bar = bar;
    order.quoteSize = quoteGained;
    order.baseSize = -baseBorrowed;
    order.price = currentPrice;
    order.isLong = false;

    double takeProfit = lastPrice * (1.0 - closeConfig.takeProfit);
    double stopLoss = lastPrice * (1.0 + closeConfig.stopLoss);
    openTrade(equity, order, takeProfit, stopLoss, closeConfig.trailingSl);

//    std::cout << "Opening Short Trade: " << std::setw(4) << getActiveTrade().toJson() << std::endl;
    return newEquity;
}

Equity Backtest::exitLong(int bar, double lastPrice, const Equity& equity, const Strategy& strategy) {
    Trade& activeTrade = getActiveTrade();
    const BrokerConfig& brokerConfig = strategy.brokerConfig;

    // If some based equity was lost to restore borrowAllowance, we take min
    double baseLost = std::min(activeTrade.openingOrder.baseSize, equity.baseEquity);

    // Price decreased due to slippage (for borrow, less borrowing)
    double currentPrice = lastPrice * (1.0 - brokerConfig.slippage);

    double quoteGained = baseLost * currentPrice;

    quoteGained = quoteGained * (1.0 - brokerConfig.commission);

    Equity newEquity = equity.getUpdatedEquity(currentPrice, quoteGained, -baseLost, 0.0);

    Order order{};
    order.bar = bar;
    order.quoteSize = quoteGained;
    order.baseSize = -baseLost;
    order.price = currentPrice;
    order.isLong = false;

    closeTrade(newEquity, order);
//    std::cout << "Closing Long Trade: " << std::setw(4) << activeTrade.toJson() << std::endl;
    return newEquity;
}

Equity Backtest::exitShort(int bar, double lastPrice, const Equity& equity, const Strategy& strategy) {
    Trade& activeTrade = getActiveTrade();
    const PositionOpenConfig& openConfig = strategy.positionOpenConfig;
    const PositionCloseConfig& closeConfig = strategy.positionCloseConfig;
    const BrokerConfig& brokerConfig = strategy.brokerConfig;
    const DepositConfig& depositConfig = strategy.depositConfig;

    // Find base that is borrowed
    double baseBorrowed =  -equity.baseEquity;

    // Price decreased due to slippage (for borrow, less borrowing)
    double currentPrice = lastPrice * (1.0 + brokerConfig.slippage);

    double baseToBuy = baseBorrowed / (1.0 - brokerConfig.commission);  // Need to buy more to cover for commission

    double quoteLost = baseToBuy * currentPrice;
    double baseGained = baseBorrowed; // The math works out.

    // Notice that the borrowed money is ALWAYS returned, even if that means quoteLost > currentEquity.quoteEquity
    // Also, a single loss making short trade would not mean available Quote Equity will go < 0.0
    Equity newEquity = equity.getUpdatedEquity(currentPrice, -quoteLost, baseGained, baseGained);


    assert(newEquity.baseEquity == 0.0 && "Base Equity must be zero after closing a short trade");


    Order order{};
    order.bar = bar;
    order.quoteSize = -quoteLost;
    order.baseSize = baseGained;
    order.price = currentPrice;
    order.isLong = true;

    closeTrade(newEquity, order);
//    std::cout << "Closing Short Trade: " << std::setw(4) << activeTrade.toJson() << std::endl;
    return newEquity;
}


Backtest doBackTest(const Strategy &strategy, const Dataset &dataset, const AcceptanceConfig acceptanceConfig) {
    Backtest backtest{strategy, dataset.dataSetConfig};
    Signal signal = computeSignal(strategy, dataset);
//    auto acceptanceConfig = acceptanceConfig;

    if (signal.max_possible_entries(dataset.numBars) < acceptanceConfig.minNumTrades) {
        return backtest;
    }
    const DepositConfig& depositConfig = strategy.depositConfig;
    const PositionOpenConfig& positionOpenConfig = strategy.positionOpenConfig;
    const PositionCloseConfig& positionCloseConfig = strategy.positionCloseConfig;
    const BrokerConfig& brokerConfig = strategy.brokerConfig;

    double borrowAllowance = depositConfig.maxBaseBorrow > 0 ? depositConfig.maxBaseBorrow : 100 * ( depositConfig.quoteDeposit / dataset.open[0]);
    Equity currentEquity = Equity(depositConfig.quoteDeposit,
                                  0.0,
                                  depositConfig.quoteDeposit,
                                  borrowAllowance);

    double minQuoteBalance = acceptanceConfig.minTotalEquityFraction * strategy.depositConfig.quoteDeposit;


    for(int bar=0; bar < dataset.numBars; bar++){
        double currentPrice = dataset.open[bar];
        if(backtest.hasActiveTrade()){
            Trade& activeTrade = backtest.getActiveTrade();

            // update the stopLoss with the current price
            if(positionCloseConfig.trailingSl){
                if(activeTrade.is_long()){
                    activeTrade.stopLoss = std::max(activeTrade.stopLoss, currentPrice * (1.0 - positionCloseConfig.stopLoss));
                } else {
                    activeTrade.stopLoss = std::min(activeTrade.stopLoss, currentPrice * (1.0 + positionCloseConfig.stopLoss));
                }
            }

            // Always close the trade on the final bar
            if(activeTrade.is_long()){
                if(signal.shouldLongExit[bar] or currentPrice <= activeTrade.stopLoss or bar == dataset.numBars-1){
                    currentEquity = backtest.exitLong(bar, currentPrice, currentEquity, strategy);
                }
            }else {
                if(signal.shouldShortExit[bar] or currentPrice >= activeTrade.stopLoss or bar == dataset.numBars-1){
                    currentEquity = backtest.exitShort(bar, currentPrice, currentEquity, strategy);
                }
            }
        }

        if(not backtest.hasActiveTrade() and bar < dataset.numBars-1){ // Don't open trade on last bar
            if(signal.shouldLongEnter[bar]){
                currentEquity = backtest.enterLong(bar, currentPrice, currentEquity, strategy);
            } else if(signal.shouldShortEnter[bar] and positionOpenConfig.bidirectional){
                currentEquity = backtest.enterShort(bar, currentPrice, currentEquity, strategy);
            }
        }

        currentEquity = currentEquity.getUpdatedEquity(currentPrice);
        backtest.equityCurve.emplace_back(currentEquity);
        if(currentEquity.totalInQuote < minQuoteBalance ){
            break;
        }
    }

    int numTrades = (int)backtest.trades.size();
    if(numTrades >= acceptanceConfig.minNumTrades && currentEquity.totalInQuote >= minQuoteBalance){
        backtest.computeMetrics(dataset);
    }

    return backtest;
}