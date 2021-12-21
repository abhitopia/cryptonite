//
// Created by Abhishek Aggarwal on 20/12/2021.
//

#include "config.h"

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

bool TradeSizeGenConfig::is_bidirectional() const {
    if(bidirectionalTradePolicy == Policy::NEVER || (bidirectionalTradePolicy == Policy::SOMETIMES && cryptonite::rand() <= 0.5)){
        return false;
    }
    return true;
}

std::tuple<bool, double> TradeSizeGenConfig::get_trade_size() const {
    if(absoluteTradeSizePolicy == Policy::NEVER || (absoluteTradeSizePolicy == Policy::SOMETIMES && cryptonite::rand() <= 0.5)){
        return {false, 1.0};
    }
    return {true, cryptonite::rand(0.1, 0.8)};
}

json TradeSizeGenConfig::toJson() {
    json j;
    j["absolute_size_policy"] = policyToString(absoluteTradeSizePolicy);
    j["bidirectional_trade_policy"] = policyToString(bidirectionalTradePolicy);
    return j;
}


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


json CriteriaGenConfig::toJson() {
    json j;
    j["numMaxEntryCriteria"] = numMaxEntryCriteria;
    j["numMaxExitCriteria"]= numMaxExitCriteria;
    j["exploration_prob"] = exploration_prob;
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
    j["criteriaGenConfig"] = criteriaGenConfig.toJson();
    j["takeProfitGenConfig"] = takeProfitGenConfig.toJson();
    j["stopLossGenConfig"] = stopLossGenConfig.toJson();
    j["brokerConfig"] = brokerConfig.toJson();
    j["depositConfig"] = depositConfig.toJson();
    return j;
}


