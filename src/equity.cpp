//
// Created by Abhishek Aggarwal on 15/12/2021.
//

#include "equity.h"

Equity Equity::getUpdatedEquity(double currentPrice, double quoteDelta, double baseDelta, double borrowAllowanceDelta) const {
    Equity newEquity{*this};
    newEquity.quoteEquity = quoteEquity + quoteDelta;
    newEquity.baseEquity = baseEquity + baseDelta;
    newEquity.borrowAllowance = borrowAllowance + borrowAllowanceDelta;
    newEquity.totalInQuote = newEquity.quoteEquity + newEquity.baseEquity * currentPrice;
    return newEquity;
}
