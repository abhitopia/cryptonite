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
#include "../include/json.h"
#include "config.h"

using json = nlohmann::json;




struct Criteria {
    std::vector<IndicatorConfig> configs{};

    Criteria(const std::vector<IndicatorConfig>& configs){
        this->configs = configs;
    }

    virtual std::shared_ptr<bool[]> reduce(int num_bars, const std::vector<std::shared_ptr<bool []>> &trig_outputs) const = 0;

    std::shared_ptr<bool[]> apply(const Dataset &dataset, bool contra=false) const;

    static std::vector<IndicatorConfig> generate_configs(int num_indicators, double exploration_prob=0.5);
    json toJson() const;
};

struct EntryCriteria : Criteria {
    explicit EntryCriteria(const std::vector<IndicatorConfig> &configs): Criteria(configs){};

    std::shared_ptr<bool[]> reduce(int num_bars, const std::vector<std::shared_ptr<bool []>> &trig_outputs) const override;

    static EntryCriteria generate(int num_indicators, double exploration_prob=0.5);
    static EntryCriteria fromJson(json j);
};

struct ExitCriteria : Criteria {
    explicit ExitCriteria(const std::vector<IndicatorConfig> &configs): Criteria(configs){};

    std::shared_ptr<bool[]> reduce(int num_bars, const std::vector<std::shared_ptr<bool []>> &trig_outputs) const override;

    static ExitCriteria generate(int num_indicators, double exploration_prob=0.5);
    static ExitCriteria fromJson(json j);
};


struct PositionOpenConfig {
    double quoteSize{1.0};
    bool isAbsolute{false};
    bool bidirectional{true};

    PositionOpenConfig(double quote_size = 1.0, bool is_absolute = false, bool bidirectional = true);

    json toJson() const;
    static PositionOpenConfig fromJson(json j);
};


struct PositionCloseConfig {
    bool trailingSl{true};
    double takeProfit{INFINITY};
    double stopLoss{INFINITY};
    PositionCloseConfig(double tp = INFINITY, double sl = INFINITY, bool trailing_sl = true);

    json toJson() const;
    static PositionCloseConfig fromJson(json j);

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
             const BrokerConfig& brokerConfig);

    static Strategy generate(const StrategyGenConfig& config);

    json toJson() const;
    static Strategy fromJson(json j);
};


#endif //CRYPTONITE_STRATEGY_H
