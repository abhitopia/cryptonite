//
// Created by Abhishek Aggarwal on 05/11/2021.
//

#include "strategy.h"

double TakeProfitGenConfig::get_tp() const {
    if(policy == Policy::NEVER || (policy == Policy::SOMETIMES && cryptonite::rand() <= 0.5)){
        return INFINITY;
    }
    return cryptonite::rand(tp_min, tp_max);
}

TakeProfitGenConfig::TakeProfitGenConfig(Policy policy, double tp_min, double tp_max) {
    this->tp_min = tp_min;
    this->tp_max = tp_max;
    this->policy = policy;
}

json TakeProfitGenConfig::toJson() {
    json j;
    j["tp_min"] = tp_min;
    j["tp_max"] = tp_max;
    j["policy"] = policyToString(policy);
    return j;
}

StopLossGenConfig::StopLossGenConfig(Policy policy, SLType type, double sl_min, double sl_max) {
    this->sl_min = sl_min;
    this->sl_max = sl_max;
    this->policy = policy;
    this->type = type;
}

bool StopLossGenConfig::is_sl_trailing() const {
    return type == SLType::TRAILING || (type == SLType::ANY && cryptonite::rand() <= 0.5);
}

double StopLossGenConfig::get_sl() const {
    if(policy == Policy::NEVER || (policy == Policy::SOMETIMES && cryptonite::rand() <= 0.5)){
        return INFINITY;
    }
    return cryptonite::rand(sl_min, sl_max);
}

json StopLossGenConfig::toJson() {
    json j;
    j["sl_min"] = sl_min;
    j["sl_max"] = sl_max;
    j["policy"] = policyToString(policy);
    j["type"] = slTypeToString(type);
    return j;
}

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


json StrategyGenConfig::toJson() {
    json j;
    j["criteriaGenConfig"] = criteriaGenConfig.toJson();
    j["takeProfitGenConfig"] = takeProfitGenConfig.toJson();
    j["stopLossGenConfig"] = stopLossGenConfig.toJson();
    return j;
}

json CriteriaGenConfig::toJson() {
    json j;
    j["numMaxEntryCriteria"] = numMaxEntryCriteria;
    j["numMaxExitCriteria"]= numMaxExitCriteria;
    j["exploration_prob"] = exploration_prob;
    return j;
}

string policyToString(Policy policy) {
    switch(policy){
        case ALWAYS: return "ALWAYS";
        case NEVER: return "NEVER";
        case SOMETIMES: return "SOMETIMES";
    }
}

string slTypeToString(SLType sl_type) {
    switch(sl_type){
        case FIXED: return "FIXED";
        case TRAILING: return "TRAILING";
        case ANY: return "ANY";
    }
}


json BrokerConfig::toJson() {
    json j;
    j["commission"] = commission;
    j["slippage"] = slippage;
    return j;
}

json DepositConfig::toJson() {
    json j;
    j["quoteDeposit"] = quoteDeposit;
    j["maxBaseBorrow"] = maxBaseBorrow;
    return j;
}

json PositionOpenConfig::toJson() {
    json j;
    j["isAbsolute"] = isAbsolute;
    j["quoteSize"] = quoteSize;
    j["bidirectional"] = bidirectional;
    return j;
}

json PositionCloseConfig::toJson() {
    json j;
    j["takeProfit"] = takeProfit;
    j["stopLoss"] = stopLoss;
    j["trailingSl"] = trailingSl;
    return j;
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

Strategy Strategy::generate(const StrategyGenConfig &config, const PositionOpenConfig &positionOpenConfig,
                            const DepositConfig &depositConfig, const BrokerConfig &brokerConfig) {
    double tp = config.takeProfitGenConfig.get_tp();
    double sl = config.stopLossGenConfig.get_sl();
    bool trailing_sl = isinf(sl) ? false : config.stopLossGenConfig.is_sl_trailing();

    PositionCloseConfig pos_close_config{tp, sl, trailing_sl};

//    int min_entry = config.criteriaGenConfig.numMaxEntryCriteria;
//    int min_exit = config.criteriaGenConfig.numMaxEntryCriteria;

    int num_entry_rules = cryptonite::randint(1, config.criteriaGenConfig.numMaxEntryCriteria + 1);
    int num_exit_rules = cryptonite::randint(1, config.criteriaGenConfig.numMaxExitCriteria + 1);

    EntryCriteria entry_criteria{EntryCriteria::generate(num_entry_rules, config.criteriaGenConfig.exploration_prob)};
    ExitCriteria exit_criteria{ExitCriteria::generate(num_exit_rules, config.criteriaGenConfig.exploration_prob)};

    return Strategy(positionOpenConfig, pos_close_config, entry_criteria, exit_criteria, depositConfig, brokerConfig);
}
