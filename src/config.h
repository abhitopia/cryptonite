//
// Created by Abhishek Aggarwal on 20/12/2021.
//

#ifndef CRYPTONITE_CONFIG_H
#define CRYPTONITE_CONFIG_H
#include <string>
#include "random.h"
#include "../include/json.h"
#include "binance.h"
#include "constants.h"
#include <tuple>


using json = nlohmann::json;

struct Strategy;

enum Policy {
    ALWAYS = 0,
    NEVER = 1,
    SOMETIMES = 2
};

std::string policyToString(Policy policy);

Policy stringToPolicy(std::string str);

enum SLType {
    FIXED = 0,
    TRAILING = 1,
    EITHER = 2
};

std::string slTypeToString(SLType sl_type);
SLType stringToSlType(std::string str);

struct TradeSizeGenConfig {
    Policy bidirectionalTradePolicy{Policy::ALWAYS};
    Policy fixedTradeSizePolicy{Policy::NEVER};

    TradeSizeGenConfig(Policy bidirectionalTradePolicy = Policy::ALWAYS, Policy fixedTradeSizePolicy = Policy::NEVER){
        this->bidirectionalTradePolicy = bidirectionalTradePolicy;
        this->fixedTradeSizePolicy = fixedTradeSizePolicy;
    }

    bool is_bidirectional() const;;

    std::tuple<bool, double> get_trade_size() const;

    json toJson() const;
    static TradeSizeGenConfig fromJson(json j);
};

struct TakeProfitGenConfig {
    Policy policy{Policy::SOMETIMES};
    double tpMin{0.01};
    double tpMax{0.1};

    TakeProfitGenConfig(Policy policy=Policy::SOMETIMES, double tpMin=0.01, double tpMax=0.1);
    double get_tp() const;

    json toJson() const;
    static TakeProfitGenConfig fromJson(json j);
};

struct StopLossGenConfig {
    Policy policy{Policy::ALWAYS};
    SLType type{SLType::EITHER};
    double slMin{0.01};
    double slMax{0.1};

    StopLossGenConfig(Policy policy=Policy::SOMETIMES, SLType type=SLType::EITHER, double sl_min=0.01, double sl_max=0.1);

    bool is_sl_trailing() const;
    double get_sl() const;
    json toJson() const;
    static StopLossGenConfig fromJson(json j);
};

struct RulesGenConfig {
    int numMaxEntryRules{4};
    int numMaxExitRules{2};
    double explorationProb{0.5};
    json toJson() const;
    static RulesGenConfig fromJson(json j);
};


struct BrokerConfig {
    double commission{0.002};
    double slippage{0.005};
    json toJson() const;
    static BrokerConfig fromJson(json j);
};

struct DepositConfig {
    double quoteDeposit{1000.0};
    double maxBaseBorrow{-1.0};
    DepositConfig(double quote_deposit=1000.0, double max_base_borrow=-1){
        this->quoteDeposit = quote_deposit;
        if(max_base_borrow < 0){
            max_base_borrow = -1.0;
        }
        this->maxBaseBorrow = max_base_borrow;
    }

    json toJson() const;
    static DepositConfig fromJson(json j);

};


struct DataSetConfig {
    std::string baseAsset;
    std::string quoteAsset;
    Interval interval;

    DataSetConfig(std::string baseAsset = "BTC", std::string quoteAsset = "USDT", Interval interval = Interval::MINUTE1){
        this->baseAsset = baseAsset;
        this->quoteAsset = quoteAsset;
        this->interval = interval;
    }

    std::string symbol();
    int intervalInSeconds();
    std::string intervalInString() const;
    bool check_valid();
    json exchangeInfo();
    json toJson() const;
    static DataSetConfig fromJson(json j);
};


struct AcceptanceConfig {
    int minNumTrades;
    double minTotalEquityFraction;
    AcceptanceConfig(int minNumTrades=500, double minTotalEquityFraction=0.5){
        assert(minNumTrades >= 500 && "Minimum number of trades less than 500 is not statistically significant");
        assert(minTotalEquityFraction < 1.0 && minTotalEquityFraction > 0);
        this->minNumTrades = minNumTrades;
        this->minTotalEquityFraction = minTotalEquityFraction;
    }

    json toJson() const;
    static AcceptanceConfig fromJson(json j);
};

struct StrategyGenConfig {
    AcceptanceConfig acceptanceConfig{};
    DataSetConfig dataSetConfig{};
    TradeSizeGenConfig tradeSizeGenConfig{};
    RulesGenConfig rulesGenConfig{};
    TakeProfitGenConfig takeProfitGenConfig{};
    StopLossGenConfig stopLossGenConfig{};
    BrokerConfig brokerConfig{};
    DepositConfig depositConfig{};
    json toJson() const;

    static StrategyGenConfig fromJson(json j);
};


#endif //CRYPTONITE_CONFIG_H
