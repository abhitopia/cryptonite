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

Policy stringToPolicy(std::string str) {
    switch(switchHash(str.c_str())){
        case switchHash("always"): return Policy::ALWAYS;
        case switchHash("never"): return Policy::NEVER;
        case switchHash("sometimes"): return Policy::SOMETIMES;
    }
}

std::string slTypeToString(SLType sl_type) {
    switch(sl_type){
        case FIXED: return "fixed";
        case TRAILING: return "trailing";
        case EITHER: return "either";
    }
}

SLType stringToSlType(std::string str) {
    switch(switchHash(str.c_str())){
        case switchHash("fixed"): return SLType::FIXED;
        case switchHash("trailing"): return SLType::TRAILING;
        case switchHash("either"): return SLType::EITHER;
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
    j["bidirectionalTradePolicy"] = policyToString(bidirectionalTradePolicy);
    j["fixedTradeSizePolicy"] = policyToString(fixedTradeSizePolicy);
    return j;
}

TradeSizeGenConfig TradeSizeGenConfig::fromJson(json j) {
    return TradeSizeGenConfig(
            stringToPolicy(j["bidirectionalTradePolicy"].get<std::string>()),
            stringToPolicy(j["fixedTradeSizePolicy"].get<std::string>())
    );
}


double TakeProfitGenConfig::get_tp() const {
    if(policy == Policy::NEVER || (policy == Policy::SOMETIMES && cryptonite::rand() <= 0.5)){
        return INFINITY;
    }
    return cryptonite::rand(tpMin, tpMax);
}

TakeProfitGenConfig::TakeProfitGenConfig(Policy policy, double tpMin, double tpMax) {
    this->tpMin = tpMin;
    this->tpMax = tpMax;
    this->policy = policy;
}

json TakeProfitGenConfig::toJson() {
    json j;
    j["tpMin"] = tpMin;
    j["tpMax"] = tpMax;
    j["policy"] = policyToString(policy);
    return j;
}

TakeProfitGenConfig TakeProfitGenConfig::fromJson(json j) {
    return TakeProfitGenConfig(
                stringToPolicy(j["policy"].get<std::string>()),
                j["tpMin"].get<double>(),
                j["tpMax"].get<double>()
            );
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

StopLossGenConfig StopLossGenConfig::fromJson(json j) {
    return StopLossGenConfig(
                stringToPolicy(j["policy"].get<std::string>()),
                stringToSlType(j["type"].get<std::string>()),
                j["slMin"].get<double>(),
                j["slMax"].get<double>());
}


json RulesGenConfig::toJson() {
    json j;
    j["numMaxEntryRules"] = numMaxEntryRules;
    j["numMaxExitRules"]= numMaxExitRules;
    j["explorationProb"] = explorationProb;
    return j;
}

RulesGenConfig RulesGenConfig::fromJson(json j) {
    auto config = RulesGenConfig();
    config.numMaxEntryRules = j["numMaxEntryRules"].get<int>();
    config.numMaxExitRules = j["numMaxExitRules"].get<int>();
    config.explorationProb = j["explorationProb"].get<double>();
    return config;
}

json BrokerConfig::toJson() {
    json j;
    j["commission"] = commission;
    j["slippage"] = slippage;
    return j;
}

BrokerConfig BrokerConfig::fromJson(json j) {
    auto config = BrokerConfig{};
    config.commission = j["commission"].get<double>();
    config.slippage = j["slippage"].get<double>();
    return config;
}

json DepositConfig::toJson() {
    json j;
    j["quoteDeposit"] = quoteDeposit;
    j["maxBaseBorrow"] = maxBaseBorrow;
    return j;
}

DepositConfig DepositConfig::fromJson(json j) {
    return DepositConfig(j["quoteDeposit"].get<int>(),
                        j["maxBaseBorrow"].get<int>());
}

json StrategyGenConfig::toJson() {
    json j;
    j["acceptanceConfig"] = acceptanceConfig.toJson();
    j["dataSetConfig"] = dataSetConfig.toJson();
    j["tradeSizeGenConfig"] = tradeSizeGenConfig.toJson();
    j["rulesGenConfig"] = rulesGenConfig.toJson();
    j["takeProfitGenConfig"] = takeProfitGenConfig.toJson();
    j["stopLossGenConfig"] = stopLossGenConfig.toJson();
    j["brokerConfig"] = brokerConfig.toJson();
    j["depositConfig"] = depositConfig.toJson();
    return j;
}

StrategyGenConfig StrategyGenConfig::fromJson(json j) {
    auto config = StrategyGenConfig();
    config.acceptanceConfig = AcceptanceConfig::fromJson(j["acceptanceConfig"]);
    config.dataSetConfig = DataSetConfig::fromJson(j["dataSetConfig"]);
    config.tradeSizeGenConfig = TradeSizeGenConfig::fromJson(j["tradeSizeGenConfig"]);
    config.rulesGenConfig = RulesGenConfig::fromJson(j["rulesGenConfig"]);
    config.takeProfitGenConfig = TakeProfitGenConfig::fromJson(j["takeProfitGenConfig"]);
    config.stopLossGenConfig = StopLossGenConfig::fromJson(j["stopLossGenConfig"]);
    config.brokerConfig = BrokerConfig::fromJson(j["brokerConfig"]);
    config.depositConfig = DepositConfig::fromJson(j["depositConfig"]);
    return config;
}


std::string DataSetConfig::symbol() {
    return baseAsset + quoteAsset;
}

int DataSetConfig::intervalInSeconds() {
    return intervalToSeconds(interval);
}

std::string DataSetConfig::intervalInString() {
    return intervalToString(interval);
}

bool DataSetConfig::check_valid() {
    // check if symbol exists using Binance API and that trading is permitted.
    auto api =  BinanceAPI{};
    auto exchangeInfo = api.getExchangeInfo();
    std::string symbol = this->symbol();
    for(auto& symbolInfo: exchangeInfo["symbols"]){
        if(symbolInfo["symbol"].get<std::string>() == symbol){
            if(symbolInfo["baseAsset"].get<std::string>() == baseAsset and symbolInfo["quoteAsset"].get<std::string>() == quoteAsset){
                return true;
            }
        }
    }
    return false;
}

json DataSetConfig::exchangeInfo() {
    auto api =  BinanceAPI{};
    return api.getExchangeInfo(this->symbol());
}

json DataSetConfig::toJson() {
    json j;
    j["baseAsset"] = baseAsset;
    j["quoteAsset"] = quoteAsset;
    j["interval"] = intervalInString();
    return j;
}

DataSetConfig DataSetConfig::fromJson(json j) {
    return DataSetConfig(j["baseAsset"].get<std::string>(),
                        j["quoteAsset"].get<std::string>(),
                         stringToInterval(j["interval"].get<std::string>()));
}

json AcceptanceConfig::toJson() {
    json j;
    j["minNumTrades"] = minNumTrades;
    j["minTotalEquityFraction"] = minTotalEquityFraction;
    return j;
}

AcceptanceConfig AcceptanceConfig::fromJson(json j) {
    return AcceptanceConfig(
            j["minNumTrades"].get<int>(),
                    j["minTotalEquityFraction"].get<double>()
            );
}

