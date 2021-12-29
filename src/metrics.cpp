//
// Created by Abhishek Aggarwal on 15/12/2021.
//

#include "metrics.h"

void Metrics::compute(const Strategy &strategy, const Dataset &dataset, const std::vector<Equity>& equityCurve, int numberOfTrades) {

    if(equityCurve.size() == 0) return;

    numTrades = numberOfTrades;
    totalReturn = equityCurve.back().totalInQuote / equityCurve[0].totalInQuote - 1.0;

    std::shared_ptr<double[]> cumProdReturns(new double [dataset.num_bars]);
    std::shared_ptr<double[]> drawDown(new double [dataset.num_bars]);
    double cumReturnsMax = 0.0, sumProfits = 0.0, sumLosses = 0.0;
    maxDrawDown = 0.0;
    avgDrawDown = 0.0;
    cumProdReturns[0] = 1.0;
    for(int i=1; i<equityCurve.size(); i++){
        double r = equityCurve[i].totalInQuote/equityCurve[i-1].totalInQuote - 1.0;

        if(r >= 0){
            sumProfits += r;
        } else {
            sumLosses += -r;
        }
        cumProdReturns[i] = (1.0 + r) * cumProdReturns[i - 1];
        cumReturnsMax = std::max(cumReturnsMax, cumProdReturns[i]);
        drawDown[i] = (cumProdReturns[i] / cumReturnsMax) - 1.0;
        maxDrawDown = std::max(maxDrawDown, -drawDown[i]);
        avgDrawDown += -drawDown[i];
    }

    profitFactor = sumProfits / sumLosses;
    avgDrawDown /= equityCurve.size();

    double numDays = dataset.durationDays();
    CAGR = std::pow((equityCurve.back().totalInQuote/equityCurve.front().totalInQuote), (365.0/numDays))  - 1.0;
    CAGROverAvgDrawDown = CAGR / avgDrawDown;
    CAGROverMaxDrawDown = CAGR / maxDrawDown;
}
