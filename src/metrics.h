//
// Created by Abhishek Aggarwal on 15/12/2021.
//

#ifndef CRYPTONITE_METRICS_H
#define CRYPTONITE_METRICS_H


#include <cmath>
#include "dataset.h"
#include "strategy.h"
#include "equity.h"

struct Metrics {
    double CAGR{-INFINITY};
    double CAGROverAvgDrawDown{-INFINITY};
    double CAGROverMaxDrawDown{-INFINITY};
    double totalReturn{-INFINITY};
    double profitFactor{-INFINITY};
    double maxDrawDown{INFINITY};
    double avgDrawDown{INFINITY};
    int numTrades{0};

    void compute(const Strategy &strategy, const Dataset &dataset, const vector<Equity>& equityCurve, int numberOfTrades);

};


#endif //CRYPTONITE_METRICS_H
