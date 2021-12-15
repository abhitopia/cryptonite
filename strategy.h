//
// Created by Abhishek Aggarwal on 05/11/2021.
//

#ifndef CRYPTONITE_STRATEGY_H
#define CRYPTONITE_STRATEGY_H

#include <iostream>
#include <string>
#include "random.h"
#include "indicator.h"
#include <math.h>
#include "lib/json/json.h"

using json = nlohmann::json;

using namespace std;

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


struct Criteria {
    vector<IndicatorConfig> configs{};

    Criteria(const vector<IndicatorConfig>& configs){
        this->configs = configs;
    }

    virtual shared_ptr<bool[]> reduce(int num_bars, const vector<shared_ptr<bool []>> &trig_outputs) const = 0;

    shared_ptr<bool[]> apply(const Dataset &dataset, bool contra=false) const;

    static vector<IndicatorConfig> generate_configs(int num_indicators, double exploration_prob=0.5);
    json toJson();
};

struct EntryCriteria : Criteria {
    explicit EntryCriteria(const vector<IndicatorConfig> &configs): Criteria(configs){};

    shared_ptr<bool[]> reduce(int num_bars, const vector<shared_ptr<bool []>> &trig_outputs) const override;

    static EntryCriteria generate(int num_indicators, double exploration_prob=0.5);

};

struct ExitCriteria : Criteria {
    explicit ExitCriteria(const vector<IndicatorConfig> &configs): Criteria(configs){};

    shared_ptr<bool[]> reduce(int num_bars, const vector<shared_ptr<bool []>> &trig_outputs) const override;

    static ExitCriteria generate(int num_indicators, double exploration_prob=0.5);

};

struct CriteriaGenConfig {
    int numMaxEntryCriteria{4};
    int numMaxExitCriteria{2};
    double exploration_prob{0.5};
    json toJson();
};


struct StrategyGenConfig {
    CriteriaGenConfig criteriaGenConfig{};
    TakeProfitGenConfig takeProfitGenConfig{};
    StopLossGenConfig stopLossGenConfig{};

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


struct PositionOpenConfig {
    double quoteSize{1.0};
    bool isAbsolute{false};
    bool bidirectional{true};

    PositionOpenConfig(double quote_size = 1.0, bool is_absolute = false, bool bidirectional = true){
        this->isAbsolute = is_absolute;
        this->quoteSize = quote_size;
        this->bidirectional = bidirectional;
        if(is_absolute) {
            assert(quote_size < 0.5 && "Quote Size must be < 0.5 for absolute positions");
        }
        assert(quote_size <= 1.0 && quote_size > 0.1 && "Quote Size must be in [0.1, 1.0]");
    }

    json toJson();

};


struct PositionCloseConfig {
    bool trailingSl{true};
    double takeProfit{INFINITY};
    double stopLoss{INFINITY};
    PositionCloseConfig(double tp = INFINITY, double sl = INFINITY, bool trailing_sl = true){
        if(tp < 0) {
            tp = INFINITY;
        }
        if(sl < 0){
            sl = INFINITY;
        }
        this->takeProfit = tp;
        this->stopLoss = sl;
        this->trailingSl = trailing_sl;
    }

    json toJson();

};


struct Strategy {
    PositionOpenConfig positionOpenConfig{};
    PositionCloseConfig positionCloseConfig{};
    EntryCriteria entryCriteria;
    ExitCriteria exitCriteria;
    DepositConfig depositConfig{};
    BrokerConfig brokerConfig{};

    Strategy(const PositionOpenConfig& positionOpenConfig,
             const PositionCloseConfig& positionCloseConfig,
             const EntryCriteria& entryCriteria,
             const ExitCriteria& exitCriteria,
             const DepositConfig& depositConfig,
             const BrokerConfig& brokerConfig
             ): entryCriteria(entryCriteria.configs), exitCriteria(exitCriteria.configs) {
        this->positionCloseConfig = positionCloseConfig;
        this->positionOpenConfig = positionOpenConfig;
        this->brokerConfig = brokerConfig;
        this->depositConfig = depositConfig;
    }

    static Strategy generate(const StrategyGenConfig& config,
                             const PositionOpenConfig& positionOpenConfig = PositionOpenConfig{},
                             const DepositConfig& depositConfig = DepositConfig{},
                             const BrokerConfig& brokerConfig = BrokerConfig{});

    json toJson();
};


#endif //CRYPTONITE_STRATEGY_H
