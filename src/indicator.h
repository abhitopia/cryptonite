//
// Created by Abhishek Aggarwal on 26/10/2021.
//

#ifndef CRYPTONITE_INDICATOR_H
#define CRYPTONITE_INDICATOR_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "trigger.h"
#include <limits>

#include "random.h"
#include "dataset.h"
#include "function.h"
#include "constants.h"
#include <math.h>
#include <limits>
#include "../include/json.h"

using json = nlohmann::json;


typedef std::shared_ptr<double[]> spda_t;
using namespace CIndicator;
class Indicator;

struct IndicatorConfig{
//    Trigger *trigger;
    std::shared_ptr<Trigger> trigger{nullptr};
    std::unordered_map<std::string, double> params{};
    Indicator* indicator;

    IndicatorConfig(){};
    IndicatorConfig(std::shared_ptr<Trigger> trigger, const std::unordered_map<std::string, double> &params, Indicator* indicator){
        this->trigger.swap(trigger);
        this->indicator = indicator;
        this->params = params;
    }
    json toJson();
    std::shared_ptr<bool[]> compute(const Dataset &dataset, bool contra_trigger = false);
};

enum MAMethod { WMA, EMA, SMA, SSMA};
enum ApplyTo { OPEN, HIGH, CLOSE, LOW, MEDIAN, TYPICAL, WEIGHTED};

int sample_gaussian_int(double mean, double sigma, int min, int max);
spda_t shift(int num_bars, int offset, spda_t source, double fill_value=dNaN);
spda_t max_arrays(int num_bars, spda_t a, spda_t b);
spda_t min_arrays(int num_bars, spda_t a, spda_t b);
spda_t rolling_sum(int num_bars, spda_t source, int period);



class Indicator {
protected:
    std::string name{};
    std::unordered_map<std::string, double> defaults{};
    std::vector<std::shared_ptr<Trigger>> triggers{};
    double min_level{dNaN};
    double max_level{dNaN};

public:
    virtual std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) = 0;
    IndicatorConfig generate_config(double exploration_prob=0.5);
    std::unordered_map<std::string, double> get_random_params(double exploration_prob=0.5);
    json toJson();
    spda_t get_source(const Dataset &dataset, ApplyTo apply_to);
    spda_t apply_ma(int num_bars, double period, spda_t source, MAMethod ma_method);
    virtual bool validate_config(IndicatorConfig &config);
    void set_level_range(double min_level, double max_level);
    bool has_level() {
        for(int j=0; j<triggers.size(); j++){
            if (triggers[j]->has_level()){
                return true;
            }
        }
        return false;
    }
};

class AcceleratorOscillator: public Indicator {
public:
    AcceleratorOscillator() {
        name="AcceleratorOscillator";
        triggers = riseFallTriggers("value") +
            higherLowerThanTriggers("value", "level") +
            crossingTriggers("value", "level") +
            directionChangeTriggers("value");
        defaults["level"] =  0.0;

    };

    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;

};

class AccumulationDistribution : public Indicator {
public:
    AccumulationDistribution(){
        name="AccumulationDistribution";
        triggers = riseFallTriggers("value") +
                   directionChangeTriggers("value");
    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;

};


class ADX : public Indicator {
public:
    ADX(){
        name="ADX";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["level"] =  0.0;
        defaults["period"] =  14;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};

class Alligator : public Indicator {
public:
    Alligator(){
        name="Alligator";
        triggers = riseFallTriggers("jaws") +
                   riseFallTriggers("lips") +
                   riseFallTriggers("teeth") +
                   crossingTriggers("lips", "teeth") +
                   crossingTriggers("lips", "jaws") +
                   crossingTriggers("teeth", "jaws");

        defaults["jaws_period"] = 13.0;
        defaults["jaws_shift"] =  8.0;
        defaults["teeth_period"] =  8.0;
        defaults["teeth_shift"] =  5.0;
        defaults["lips_period"] =  5.0;
        defaults["lips_shift"] =  3.0;
        defaults["apply_to"] = (double) ApplyTo::MEDIAN;
        defaults["ma_method"] = (double) MAMethod::SSMA;
    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class AverageTrueRange : public Indicator {
public:
    AverageTrueRange(){
        name="AverageTrueRange";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["level"] =  0.01;
        defaults["period"] =  14;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class BearsPower : public Indicator {
public:
    BearsPower(){
        name="BearsPower";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["level"] =  0.00;
        defaults["period"] =  13;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};

class BullsPower : public Indicator {
public:
    BullsPower(){
        name="BullsPower";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["level"] =  0.00;
        defaults["period"] =  13;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class BollingerBands : public Indicator {
public:
    BollingerBands(){
        name="BollingerBands";
        triggers = crossingTriggers("bar", "upper") +
                   crossingTriggers("bar", "lower") +
                   std::shared_ptr<Trigger>(new HigherThan{"bar", "upper"}) +
                   std::shared_ptr<Trigger>(new LowerThan{"bar", "lower"});

        defaults["period"] =  20;
        defaults["deviation"] =  2.0;
        defaults["apply_to"] = (double) ApplyTo::CLOSE;
    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};

class CommodityChannelIndex : public Indicator {
public:
    CommodityChannelIndex(){
        name="CommodityChannelIndex";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["level"] =  100;
        defaults["period"] =  14;
        defaults["apply_to"] = (double) ApplyTo::TYPICAL;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class DeMarker : public Indicator {
public:
    DeMarker(){
        name="DeMarker";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["level"] =  0.3;
        defaults["period"] =  14;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};

class DirectionalIndicators : public Indicator {
public:
    DirectionalIndicators(){
        name="DirectionalIndicators";
        triggers = higherLowerThanTriggers("plus", "minus") +
                   crossingTriggers("plus", "minus");
        defaults["period"] =  10;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};

class DonchianChannel : public Indicator {
public:
    DonchianChannel(){
        name="DonchianChannel";
        triggers = crossingTriggers("bar", "upper") +
                   crossingTriggers("bar", "lower") +
                   std::shared_ptr<Trigger>(new HigherThan{"bar", "upper"}) +
                   std::shared_ptr<Trigger>(new LowerThan{"bar", "lower"});
        defaults["period"] =  10;
        
    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class Envelopes : public Indicator {
public:
    Envelopes(){
        name="Envelopes";
        triggers = crossingTriggers("bar", "upper") +
                   crossingTriggers("bar", "lower") +
                   std::shared_ptr<Trigger>(new HigherThan{"bar", "upper"}) +
                   std::shared_ptr<Trigger>(new LowerThan{"bar", "lower"});

        defaults["period"] = 14;
        defaults["deviation_pct"] = 10;
        defaults["ma_method"] = MAMethod::SMA;
        defaults["apply_to"] = ApplyTo::CLOSE;
    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class ForceIndex : public Indicator {
public:
    ForceIndex(){
        name="ForceIndex";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "zero") +
                   crossingTriggers("value", "zero") +
                   directionChangeTriggers("value");
        defaults["ma_method"] =  MAMethod::SMA;
        defaults["period"] =  13;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class MACD : public Indicator {
public:
    MACD(){
        name="MACD";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "zero") +
                   crossingTriggers("value", "zero") +
                   directionChangeTriggers("value");
        defaults["fast_period"] = 12;
        defaults["slow_period"] = 26;
        defaults["signal_period"] = 9;
        defaults["apply_to"] = ApplyTo::CLOSE;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
    bool validate_config(IndicatorConfig &config) override;

};


class MACDSignal : public Indicator {
public:
    MACDSignal(){
        name="MACDSignal";
        triggers = higherLowerThanTriggers("macd", "signal") +
                   crossingTriggers("macd", "signal");
        defaults["fast_period"] = 12;
        defaults["slow_period"] = 26;
        defaults["signal_period"] = 9;
        defaults["apply_to"] = ApplyTo::CLOSE;
    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
    bool validate_config(IndicatorConfig &config) override;
};


class Momentum : public Indicator {
public:
    Momentum(){
        name="Momentum";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["period"] = 14.0;
        defaults["level"] = 100.0;
        defaults["apply_to"] = ApplyTo::CLOSE;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class MoneyFlowIndex : public Indicator {
public:
    MoneyFlowIndex(){
        name="MoneyFlowIndex";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["period"] = 14.0;
        defaults["level"] = 20.0;
    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class MovingAverage : public Indicator {
public:
    MovingAverage(){
        name="MovingAverage";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "bar") +
                   crossingTriggers("value", "bar") +
                   directionChangeTriggers("value");
        defaults["period"] = 14.0;
        defaults["shift"] = 0.0;
        defaults["ma_method"] = MAMethod::SMA;
        defaults["apply_to"] = ApplyTo::CLOSE;
        
    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class MovingAverageOscillator : public Indicator {
public:
    MovingAverageOscillator(){
        name="MovingAverageOscillator";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["fast_period"] = 12;
        defaults["slow_period"] = 26;
        defaults["signal_period"] = 9;
        defaults["apply_to"] = ApplyTo::CLOSE;
        defaults["level"] = 0.0;
    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
    bool validate_config(IndicatorConfig &config) override;

};


class MovingAverageCrossOver : public Indicator {
public:
    MovingAverageCrossOver(){
        name="MovingAverageCrossOver";
        triggers = higherLowerThanTriggers("fast", "slow") +
                   crossingTriggers("fast", "slow");
        defaults["fast_period"] = 12;
        defaults["slow_period"] = 26;
        defaults["ma_method"] = MAMethod::SMA;
    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class OnBalanceVolume : public Indicator {
public:
    OnBalanceVolume(){
        name="OnBalanceVolume";
        triggers = riseFallTriggers("value") +
                   directionChangeTriggers("value");
    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class RelativeStrengthIndex : public Indicator {
public:
    RelativeStrengthIndex(){
        name="RelativeStrengthIndex";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["period"] = 14;
        defaults["apply_to"] = ApplyTo::CLOSE;
        defaults["level"] = 30.0;
        
    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class RelativeVigorIndex : public Indicator {
public:
    RelativeVigorIndex(){
        name="RelativeVigorIndex";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["period"] = 10;
        defaults["level"] = 0.0;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class RelativeVigorIndexSignal : public Indicator {
public:
    RelativeVigorIndexSignal(){
        name="RelativeVigorIndexSignal";
        triggers = higherLowerThanTriggers("rvi", "signal") +
                   crossingTriggers("rvi", "signal");
        defaults["period"] = 10;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class StandardDeviation : public Indicator {
public:
    StandardDeviation(){
        name="StandardDeviation";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["period"] = 20;
        defaults["level"] = 0.0;
        defaults["apply_to"] = ApplyTo::CLOSE;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class Stochastic : public Indicator {
public:
    Stochastic(){
        name="Stochastic";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["pct_k_period"] = 5;
        defaults["pct_k_slowing_period"] = 3;
        defaults["pct_d_period"] = 3;
        defaults["level"] = 20;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class StochasticSignal : public Indicator {
public:
    StochasticSignal(){
        name="StochasticSignal";
        triggers = higherLowerThanTriggers("stoch", "signal") +
                   crossingTriggers("stoch", "signal");
        defaults["pct_k_period"] = 5;
        defaults["pct_k_slowing_period"] = 3;
        defaults["pct_d_period"] = 3;
        
    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class Volumes : public Indicator {
public:
    Volumes(){
        name="Volumes";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["level"] = 1000;
        
    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class WilliamsPercentRange : public Indicator {
public:
    WilliamsPercentRange(){
        name="WilliamsPercentRange";
        triggers = riseFallTriggers("value") +
                   higherLowerThanTriggers("value", "level") +
                   crossingTriggers("value", "level") +
                   directionChangeTriggers("value");
        defaults["level"] = -20;
        defaults["period"] = 14;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class CandleColor : public Indicator {
public:
    CandleColor(){
        name="CandleColor";
        triggers = higherLowerThanTriggers("value", "zero");
        defaults["min_change_pct"] = 1.0;
        defaults["consecutive_period"] = 2;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class PinBar : public Indicator {
public:
    PinBar(){
        name="PinBar";
        triggers = higherLowerThanTriggers("value", "zero");
        defaults["max_body_pct"] = 8.0;
        defaults["min_wick_pct"] = 30.0;

    }
    std::unordered_map<std::string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};

extern std::vector<std::shared_ptr<Indicator>> Indicators;


void setup(const Dataset &dataset, int seed=-1);

#endif //CRYPTONITE_INDICATOR_H
