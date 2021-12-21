//
// Created by Abhishek Aggarwal on 15/12/2021.
//

#ifndef CRYPTONITE_EQUITY_H
#define CRYPTONITE_EQUITY_H


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

    Equity getUpdatedEquity(double currentPrice, double quoteDelta = 0.0, double baseDelta = 0.0, double borrowAllowanceDelta = 0.0) const;
};


#endif //CRYPTONITE_EQUITY_H
