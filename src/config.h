//
// Created by Abhishek Aggarwal on 20/12/2021.
//

#ifndef CRYPTONITE_CONFIG_H
#define CRYPTONITE_CONFIG_H
#include <string>
#include "random.h"
#include "../include/json.h"
#include <tuple>


using json = nlohmann::json;

using namespace std;

struct Strategy;

enum Policy {
    ALWAYS = 0,
    NEVER = 1,
    SOMETIMES = 2
};

string policyToString(Policy policy);

enum SLType {
    FIXED = 0,
    TRAILING = 1,
    ANY = 2
};

string slTypeToString(SLType sl_type);


struct TradeSizeGenConfig {
    Policy bidirectionalTradePolicy{Policy::ALWAYS};
    Policy absoluteTradeSizePolicy{Policy::NEVER};

    TradeSizeGenConfig(Policy bidirectionalTradePolicy = Policy::ALWAYS, Policy absoluteTradeSizePolicy = Policy::NEVER){
        this->bidirectionalTradePolicy = bidirectionalTradePolicy;
        this->absoluteTradeSizePolicy = absoluteTradeSizePolicy;
    }

    bool is_bidirectional() const;;

    std::tuple<bool, double> get_trade_size() const;

    json toJson();
};

struct TakeProfitGenConfig {
    Policy policy{Policy::SOMETIMES};
    double tp_min{0.01};
    double tp_max{0.1};

    TakeProfitGenConfig(Policy policy=Policy::SOMETIMES, double tp_min=0.01, double tp_max=0.1);
    double get_tp() const;

    json toJson();
};

struct StopLossGenConfig {
    Policy policy{Policy::ALWAYS};
    SLType type{SLType::ANY};
    double sl_min{0.01};
    double sl_max{0.1};

    StopLossGenConfig(Policy policy=Policy::SOMETIMES, SLType type=SLType::ANY, double sl_min=0.01, double sl_max=0.1);

    bool is_sl_trailing() const;
    double get_sl() const;
    json toJson();
};

struct CriteriaGenConfig {
    int numMaxEntryCriteria{4};
    int numMaxExitCriteria{2};
    double exploration_prob{0.5};
    json toJson();
};


struct BrokerConfig {
    double commission{0.002};
    double slippage{0.005};
    json toJson();
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

    json toJson();

};

struct StrategyGenConfig {
    TradeSizeGenConfig tradeSizeGenConfig{};
    CriteriaGenConfig criteriaGenConfig{};
    TakeProfitGenConfig takeProfitGenConfig{};
    StopLossGenConfig stopLossGenConfig{};
    BrokerConfig brokerConfig{};
    DepositConfig depositConfig{};
    json toJson();
};


#endif //CRYPTONITE_CONFIG_H
