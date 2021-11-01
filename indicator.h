//
// Created by Abhishek Aggarwal on 26/10/2021.
//

#ifndef CRYPTONITE_INDICATOR_H
#define CRYPTONITE_INDICATOR_H

#include <iostream>
#include <string>
#include <unordered_map>
#include "trigger.h"
#include "random.h"
#include "dataset.h"
#include "function.h"
#include "constants.h"
#include <limits>


typedef shared_ptr<double[]> spda_t;
using namespace CIndicator;
class Indicator;

struct IndicatorConfig{
    Trigger *trigger{nullptr};
    unordered_map<string, double> params{};
    Indicator *indicator{nullptr};
    void print();
};

enum MAMethod { WMA, EMA, SMA, SSMA};
enum ApplyTo { OPEN, HIGH, CLOSE, LOW, MEDIAN, TYPICAL, WEIGHTED};

int sample_gaussian_int(double mean, double sigma, int min, int max);

class Indicator {
protected:
    string name{};
    unordered_map<string, double> defaults{};
    vector<Trigger> triggers{};
public:
    virtual unordered_map<string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) = 0;
    IndicatorConfig generate_config(double exploration_prob=0.5);
    unordered_map<string, double> get_random_params(double exploration_prob=0.5);
    void permute_level(int num_bars, IndicatorConfig &config, unordered_map<string, spda_t> &indicator_output);
    void to_json();
    spda_t get_source(const Dataset &dataset, ApplyTo apply_to);
    spda_t apply_ma(int num_bars, double period, spda_t source, MAMethod ma_method);
    spda_t shift(int num_bars, int offset, spda_t source, double fill_value=dNaN);
};

class AcceleratorOscillator: public Indicator {
public:
    AcceleratorOscillator(){
        name="AcceleratorOscillator";
        triggers = riseFallTriggers("value") +
            higherLowerThanTriggers("value", "level") +
            crossingTriggers("value", "level") +
            directionChangeTriggers("value");
        defaults["level"] =  0.0;
    };

//    void check_sma(int num_bars, spda_t target, spda_t data, int num_nan, int period){
//        for(int i=0; i < num_bars; i++){
//            if(i < num_nan){
//                assert(target[i] != target[i]);
//            } else {
//                assert(target[i] == target[i]);
//            }
//        }
//
//        for(int i= num_nan; i < num_bars; i++){
//            double sum_i = 0.0;
//            for(int j=0; j < period; j++){
//                sum_i += data[i-j];
//            }
//            sum_i /= period;
//            cout << std::abs(target[i] - sum_i) << endl;
//            assert(std::abs(target[i] - sum_i) < 0.000000001);
//        }
//    }
//
//    void check_sub(int num_bars, spda_t target, spda_t first, spda_t second, int num_nan){
//        for(int i=0; i< num_bars; i++){
//            if(i < num_nan){
//                assert(target[i] != target[i]);
//            } else {
//                assert(target[i] == target[i]);
//                double src = first[i] - second[i];
//                assert(target[i] == src);
//            }
//
//        }
//    }

    unordered_map<string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;

};

class AccumulationDistribution : public Indicator {
public:
    AccumulationDistribution(){
        name="AccumulationDistribution";
        triggers = riseFallTriggers("value") +
                   directionChangeTriggers("value");
    }
    unordered_map<string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;

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
    unordered_map<string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
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
    unordered_map<string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
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
    unordered_map<string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
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
    unordered_map<string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
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
    unordered_map<string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};


class BollingerBands : public Indicator {
public:
    BollingerBands(){
        name="BollingerBands";
        triggers = crossingTriggers("bar", "upper") +
                   crossingTriggers("bar", "lower") +
                   vector<Trigger>{HigherThan{"bar", "upper"}, LowerThan{"bar", "lower"}};

        defaults["level"] =  0.00;
        defaults["deviation"] =  2.0;
        defaults["apply_to"] = (double) ApplyTo::MEDIAN;
    }
    unordered_map<string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
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
    unordered_map<string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
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
    unordered_map<string, spda_t> compute(const Dataset &dataset, const IndicatorConfig &config) override;
};

//vector<Indicator> Indicators{AcceleratorOscillator{}};


#endif //CRYPTONITE_INDICATOR_H
