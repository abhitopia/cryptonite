//
// Created by Abhishek Aggarwal on 09/12/2021.
//

#include "backtest.h"

Signal computeSignal(const Strategy &strategy, const Dataset &dataset) {
    Signal signal;
//    signal.shouldLongEnter = strategy.entryCriteria.apply(dataset, false);
//    signal.shouldLongExit = strategy.exitCriteria.apply(dataset, false);
//    if(strategy.positionOpenConfig.bidirectional){
//        signal.shouldShortEnter = strategy.entryCriteria.apply(dataset, true);
//        signal.shouldShortExit = strategy.exitCriteria.apply(dataset, true);
//    }



    #pragma omp parallel sections default(none) shared(signal, dataset, strategy)
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

    double availableQuoteEquity = std::max(0.0, equity.quoteEquity);
    double tradeSize = openConfig.isAbsolute ? openConfig.quoteSize : (openConfig.quoteSize * availableQuoteEquity);
    double quoteLost = std::min(availableQuoteEquity, tradeSize);

    if(quoteLost == 0){
        return equity;
    }

    // Price increased due to slippage
    double currentPrice = lastPrice * (1.0 + brokerConfig.slippage);
    double baseGained = quoteLost / currentPrice;

    // Commission is always applied on purchased asset
    baseGained = baseGained * (1.0 - brokerConfig.commission);

    // If there is any baseEquity that was borrowed but not returned, return it now.
    double allowanceReturned = equity.baseEquity >=0.0 ? 0.0 : -equity.baseEquity;

    Equity newEquity = equity.getUpdatedEquity(currentPrice, -quoteLost, baseGained, allowanceReturned);

    Order order;
    order.bar = bar;
    order.quoteSize = -quoteLost;
    order.baseSize = baseGained;
    order.price = currentPrice;
    order.isLong = true;

    double takeProfit = lastPrice * (1.0 + closeConfig.takeProfit);
    double stopLoss = lastPrice * (1.0 - closeConfig.stopLoss);
    openTrade(order, takeProfit, stopLoss, closeConfig.trailingSl);

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

    Order order;
    order.bar = bar;
    order.quoteSize = quoteGained;
    order.baseSize = -baseBorrowed;
    order.price = currentPrice;
    order.isLong = false;

    double takeProfit = lastPrice * (1.0 - closeConfig.takeProfit);
    double stopLoss = lastPrice * (1.0 + closeConfig.stopLoss);
    openTrade(order, takeProfit, stopLoss, closeConfig.trailingSl);
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

    Order order;
    order.bar = bar;
    order.quoteSize = quoteGained;
    order.baseSize = -baseLost;
    order.price = currentPrice;
    order.isLong = false;

    closeTrade(order);
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

    Equity newEquity = equity.getUpdatedEquity(currentPrice, -quoteLost, baseGained, baseGained);

    Order order;
    order.bar = bar;
    order.quoteSize = -quoteLost;
    order.baseSize = baseGained;
    order.price = currentPrice;
    order.isLong = true;

    closeTrade(order);
    return newEquity;
}


void Backtest::operator()(const Strategy &strategy, const Dataset &dataset, const StoppingCriteria& stoppingCriteria) {

    Signal signal = computeSignal(strategy, dataset);
    if (signal.max_possible_entries(dataset.num_bars) < stoppingCriteria.minNumTrades) {
        return;
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


    for(int bar=0; bar < dataset.num_bars; bar++){
        double currentPrice = dataset.open[bar];
        if(hasActiveTrade()){
            Trade& activeTrade = getActiveTrade();

            // update the stopLoss with the current price
            if(positionCloseConfig.trailingSl){
                if(activeTrade.is_long()){
                    activeTrade.stopLoss = std::max(activeTrade.stopLoss, currentPrice * (1.0 - positionCloseConfig.stopLoss));
                } else {
                    activeTrade.stopLoss = std::min(activeTrade.stopLoss, currentPrice * (1.0 + positionCloseConfig.stopLoss));
                }
            }

            if(activeTrade.is_long()){
                if(signal.shouldLongExit[bar] or currentPrice <= activeTrade.stopLoss){
                    currentEquity = exitLong(bar, currentPrice, currentEquity, strategy);
                }
            }else {
                if(signal.shouldShortExit[bar] or currentPrice >= activeTrade.stopLoss){
                    currentEquity = exitShort(bar, currentPrice, currentEquity, strategy);
                }
            }
        }

        if(not hasActiveTrade()){
            if(signal.shouldLongEnter[bar]){
                currentEquity = enterLong(bar, currentPrice, currentEquity, strategy);
            } else if(signal.shouldShortEnter[bar] and positionOpenConfig.bidirectional){
                currentEquity = enterShort(bar, currentPrice, currentEquity, strategy);
            }
        }

        currentEquity = currentEquity.getUpdatedEquity(currentPrice);
        equityCurve.emplace_back(currentEquity);
        if(currentEquity.totalInQuote < stoppingCriteria.minTotalEquityFraction * strategy.depositConfig.quoteDeposit ){
            break;
        }
    }

    int numTrades = trades.size();
    if(numTrades >= stoppingCriteria.minNumTrades){
        metrics.compute(strategy, dataset, equityCurve, numTrades);

    }
}
