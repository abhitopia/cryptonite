//
// Created by Abhishek Aggarwal on 05/11/2021.
//

#include "strategy.h"



shared_ptr<bool[]> Criteria::apply(const Dataset &dataset, bool contra) const {
    int num_bars = dataset.num_bars;
    vector<shared_ptr<bool[]>> trig_outputs{};
    for (int i=0; i<configs.size(); i++){
        trig_outputs.push_back(shared_ptr<bool[]>{nullptr});
    }

#pragma omp parallel for default(none) shared(trig_outputs, dataset, contra, configs)
    for(int i=0; i<configs.size(); i++){
        IndicatorConfig config = configs[i];
        auto result = config.compute(dataset, contra);
        trig_outputs[i].swap(result);
    }
    return this->reduce(num_bars, trig_outputs);
}

vector<IndicatorConfig> Criteria::generate_configs(int num_indicators, double exploration_prob) {
    int n_indicators = Indicators.size();
    vector<IndicatorConfig> configs{};
    for(int i=0; i<num_indicators;i++){
        configs.push_back(Indicators[cryptonite::randint(0, n_indicators)]->generate_config(exploration_prob));
    }
    return configs;
}

shared_ptr<bool[]> EntryCriteria::reduce(int num_bars, const vector<shared_ptr<bool[]>> &trig_outputs) const {
    shared_ptr<bool[]> result{new bool [num_bars]};

#pragma omp parallel for default(none) shared(result, num_bars, trig_outputs)
    for (int bar=0; bar < num_bars; bar++){
        result[bar] = true;
        for(int i=0; i<trig_outputs.size(); i++){
            result[bar] = result[bar] && trig_outputs[i][bar];
        }
    }
    return result;
}

EntryCriteria EntryCriteria::generate(int num_indicators, double exploration_prob) {
    return EntryCriteria(Criteria::generate_configs(num_indicators, exploration_prob));
}

json Criteria::toJson() {
    json j;
    for (auto &config: configs){
        j.push_back(config.toJson());
    }
    return j;
}

shared_ptr<bool[]> ExitCriteria::reduce(int num_bars, const vector<shared_ptr<bool[]>> &trig_outputs) const {
    shared_ptr<bool[]> result{new bool [num_bars]};

    #pragma omp parallel for default(none) shared(result, num_bars, trig_outputs)
    for (int bar=0; bar < num_bars; bar++){
        result[bar] = false;
        for(int i=0; i<trig_outputs.size(); i++){
            result[bar] = result[bar] || trig_outputs[i][bar];
        }
    }
    return result;
}

ExitCriteria ExitCriteria::generate(int num_indicators, double exploration_prob) {
    return ExitCriteria(Criteria::generate_configs(num_indicators, exploration_prob));
}

json PositionOpenConfig::toJson() {
    json j;
    j["isAbsolute"] = isAbsolute;
    j["quoteSize"] = quoteSize;
    j["bidirectional"] = bidirectional;
    return j;
}

PositionOpenConfig::PositionOpenConfig(double quote_size, bool is_absolute, bool bidirectional) {
    this->isAbsolute = is_absolute;
    this->quoteSize = quote_size;
    this->bidirectional = bidirectional;
    if(is_absolute) {
        assert(quote_size <= 0.8 && quote_size > 0.1 && "Quote Size must be in [0.1, 0.8] for absolute positions");
    } else {
        assert(quote_size == 1.0 && "Quote Size must be 1.0 for non-absolute positions");
    }

}

json PositionCloseConfig::toJson() {
    json j;
    j["takeProfit"] = takeProfit;
    j["stopLoss"] = stopLoss;
    j["trailingSl"] = trailingSl;
    return j;
}

PositionCloseConfig::PositionCloseConfig(double tp, double sl, bool trailing_sl) {
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


json Strategy::toJson() {
    json j;
    j["positionOpenConfig"] = positionOpenConfig.toJson();
    j["positionCloseConfig"] = positionCloseConfig.toJson();
    j["brokerConfig"] = brokerConfig.toJson();
    j["depositConfig"] = depositConfig.toJson();
    j["entryCriteria"] = entryCriteria.toJson();
    j["exitCriteria"] = exitCriteria.toJson();
    return j;
}

Strategy::Strategy(const PositionOpenConfig &positionOpenConfig, const PositionCloseConfig &positionCloseConfig,
                   const EntryCriteria &entryCriteria, const ExitCriteria &exitCriteria,
                   const DepositConfig &depositConfig, const BrokerConfig &brokerConfig) : entryCriteria(entryCriteria.configs), exitCriteria(exitCriteria.configs) {
    this->positionCloseConfig = positionCloseConfig;
    this->positionOpenConfig = positionOpenConfig;
    this->brokerConfig = brokerConfig;
    this->depositConfig = depositConfig;
}

Strategy Strategy::generate(const StrategyGenConfig &config) {
    double tp = config.takeProfitGenConfig.get_tp();
    double sl = config.stopLossGenConfig.get_sl();
    bool trailingSl = isinf(sl) ? false : config.stopLossGenConfig.is_sl_trailing();
    bool isBidirectional = config.tradeSizeGenConfig.is_bidirectional();
    auto [isAbsolute, quoteSize] = config.tradeSizeGenConfig.get_trade_size();

    PositionOpenConfig positionOpenConfig{quoteSize, isAbsolute, isBidirectional};
    PositionCloseConfig positionCloseConfig{tp, sl, trailingSl};

    int num_entry_rules = cryptonite::randint(1, config.rulesGenConfig.numMaxEntryRules + 1);
    int num_exit_rules = cryptonite::randint(1, config.rulesGenConfig.numMaxExitRules + 1);

    EntryCriteria entry_criteria{EntryCriteria::generate(num_entry_rules, config.rulesGenConfig.explorationProb)};
    ExitCriteria exit_criteria{ExitCriteria::generate(num_exit_rules, config.rulesGenConfig.explorationProb)};

    return Strategy(positionOpenConfig,
                    positionCloseConfig,
                    entry_criteria,
                    exit_criteria,
                    config.depositConfig,
                    config.brokerConfig);
}


