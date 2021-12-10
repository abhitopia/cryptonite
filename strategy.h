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
     double get_tp();

     json toJson();
};

struct StopLossGenConfig {
    Policy policy{Policy::ALWAYS};
    SLType type{SLType::ANY};
    double sl_min{0.01};
    double sl_max{0.1};

    StopLossGenConfig(Policy policy=Policy::SOMETIMES, SLType type=SLType::ANY, double sl_min=0.01, double sl_max=0.1);

    bool is_sl_trailing();
    double get_sl();
    json toJson();
};


struct Criteria {
    vector<IndicatorConfig> configs{};

    Criteria(const vector<IndicatorConfig> &configs){
        this->configs = configs;
    }

    virtual shared_ptr<bool[]> reduce(int num_bars, const vector<shared_ptr<bool []>> &trig_outputs) = 0;

    shared_ptr<bool[]> apply(const Dataset &dataset, bool contra=false);

    static vector<IndicatorConfig> generate_configs(int num_indicators, double exploration_prob=0.5);
};

struct EntryCriteria : Criteria {
    explicit EntryCriteria(const vector<IndicatorConfig> &configs): Criteria(configs){};

    shared_ptr<bool[]> reduce(int num_bars, const vector<shared_ptr<bool []>> &trig_outputs) override;

    static EntryCriteria generate(int num_indicators, double exploration_prob=0.5);

    void toJson(){

    }
};

struct ExitCriteria : Criteria {
    explicit ExitCriteria(const vector<IndicatorConfig> &configs): Criteria(configs){};

    shared_ptr<bool[]> reduce(int num_bars, const vector<shared_ptr<bool []>> &trig_outputs) override;

    static ExitCriteria generate(int num_indicators, double exploration_prob=0.5);
};

struct CriteriaGenConfig {
    int num_max_entry_criteria{4};
    int num_max_exit_criteria{2};
    double exploration_prob{0.5};
    json toJson();
};

struct StrategyGenConfig {
    CriteriaGenConfig crit_gen_config{};
    TakeProfitGenConfig tp_gen_config{};
    StopLossGenConfig sl_gen_config{};

    json toJson();
};

struct Strategy {
    bool trailing_sl{};
    double tp{INFINITY};
    double sl{INFINITY};
    EntryCriteria entryCriteria;
    ExitCriteria exitCriteria;

    Strategy(double tp, double sl, bool trailing_sl, const EntryCriteria& entryCriteria, const ExitCriteria& exitCriteria): entryCriteria(entryCriteria.configs), exitCriteria(exitCriteria.configs) {
        this->tp = tp;
        this->sl = sl;
        this->trailing_sl = trailing_sl;
    }

    static Strategy generate(StrategyGenConfig config){
        double tp = config.tp_gen_config.get_tp();
        bool trailing_sl = config.sl_gen_config.is_sl_trailing();
        double sl = config.sl_gen_config.get_sl();

        int num_entry_rules = cryptonite::randint(1, config.crit_gen_config.num_max_entry_criteria + 1);
        int num_exit_rules = cryptonite::randint(1, config.crit_gen_config.num_max_exit_criteria + 1);

        EntryCriteria entry_criteria{EntryCriteria::generate(num_entry_rules, config.crit_gen_config.exploration_prob)};
        ExitCriteria exit_criteria{ExitCriteria::generate(num_exit_rules, config.crit_gen_config.exploration_prob)};

        return Strategy(tp, sl, trailing_sl, entry_criteria, exit_criteria);
    }
};


#endif //CRYPTONITE_STRATEGY_H
