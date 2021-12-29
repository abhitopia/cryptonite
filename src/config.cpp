//
// Created by Abhishek Aggarwal on 20/12/2021.
//

#include "config.h"

std::string policyToString(Policy policy) {
    switch(policy){
        case ALWAYS: return "always";
        case NEVER: return "never";
        case SOMETIMES: return "sometimes";
    }
}

std::string slTypeToString(SLType sl_type) {
    switch(sl_type){
        case FIXED: return "fixed";
        case TRAILING: return "trailing";
        case EITHER: return "either";
    }
}

bool TradeSizeGenConfig::is_bidirectional() const {
    if(bidirectionalTradePolicy == Policy::NEVER || (bidirectionalTradePolicy == Policy::SOMETIMES && cryptonite::rand() <= 0.5)){
        return false;
    }
    return true;
}

std::tuple<bool, double> TradeSizeGenConfig::get_trade_size() const {
    if(fixedTradeSizePolicy == Policy::NEVER || (fixedTradeSizePolicy == Policy::SOMETIMES && cryptonite::rand() <= 0.5)){
        return {false, 1.0};
    }
    return {true, cryptonite::rand(0.1, 0.8)};
}

json TradeSizeGenConfig::toJson() {
    json j;
    j["absolute_size_policy"] = policyToString(fixedTradeSizePolicy);
    j["bidirectional_trade_policy"] = policyToString(bidirectionalTradePolicy);
    return j;
}


double TakeProfitGenConfig::get_tp() const {
    if(policy == Policy::NEVER || (policy == Policy::SOMETIMES && cryptonite::rand() <= 0.5)){
        return INFINITY;
    }
    return cryptonite::rand(tpMin, tpMax);
}

TakeProfitGenConfig::TakeProfitGenConfig(Policy policy, double tp_min, double tp_max) {
    this->tpMin = tp_min;
    this->tpMax = tp_max;
    this->policy = policy;
}

json TakeProfitGenConfig::toJson() {
    json j;
    j["tpMin"] = tpMin;
    j["tpMax"] = tpMax;
    j["policy"] = policyToString(policy);
    return j;
}

StopLossGenConfig::StopLossGenConfig(Policy policy, SLType type, double sl_min, double sl_max) {
    this->slMin = sl_min;
    this->slMax = sl_max;
    this->policy = policy;
    this->type = type;
}

bool StopLossGenConfig::is_sl_trailing() const {
    return type == SLType::TRAILING || (type == SLType::EITHER && cryptonite::rand() <= 0.5);
}

double StopLossGenConfig::get_sl() const {
    if(policy == Policy::NEVER || (policy == Policy::SOMETIMES && cryptonite::rand() <= 0.5)){
        return INFINITY;
    }
    return cryptonite::rand(slMin, slMax);
}

json StopLossGenConfig::toJson() {
    json j;
    j["slMin"] = slMin;
    j["slMax"] = slMax;
    j["policy"] = policyToString(policy);
    j["type"] = slTypeToString(type);
    return j;
}


json RulesGenConfig::toJson() {
    json j;
    j["numMaxEntryRules"] = numMaxEntryRules;
    j["numMaxExitRules"]= numMaxExitRules;
    j["explorationProb"] = explorationProb;
    return j;
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

json StrategyGenConfig::toJson() {
    json j;
    j["tradeSizeGenConfig"] = tradeSizeGenConfig.toJson();
    j["rulesGenConfig"] = rulesGenConfig.toJson();
    j["takeProfitGenConfig"] = takeProfitGenConfig.toJson();
    j["stopLossGenConfig"] = stopLossGenConfig.toJson();
    j["brokerConfig"] = brokerConfig.toJson();
    j["depositConfig"] = depositConfig.toJson();
    return j;
}


