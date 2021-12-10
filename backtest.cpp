//
// Created by Abhishek Aggarwal on 09/12/2021.
//

#include "backtest.h"

void backtest(Strategy &strategy, const Dataset &dataset) {
    auto longEnter = strategy.entryCriteria.apply(dataset, false);
    auto longExit = strategy.entryCriteria.apply(dataset, true);
    auto shortEnter = strategy.exitCriteria.apply(dataset, false);
    auto shortExit = strategy.exitCriteria.apply(dataset, true);
}
