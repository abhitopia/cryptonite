//
// Created by Abhishek Aggarwal on 26/10/2021.
//

#include "indicator.h"


int sample_gaussian_int(double mean, double sigma, int min, int max) {
    int result{};
    while (true){
        result = (int)cryptonite::randn(mean, sigma);
        if ( min <= result && result <= max){
            return result;
        }
    }
}


void Indicator::to_json() {
    std::cout << "Indicator: " << name << endl;
}

unordered_map<string, double> Indicator::get_random_params(double exploration_prob) {
    unordered_map<string, double> params;
    for (auto const& [key, val] : defaults)
    {
        if (key == "level" || cryptonite::rand() > exploration_prob){ // level or don't explore
            params[key] = val;
        } else if (key == "apply_to"){
            params[key] = cryptonite::randint(0, 7);
        } else if (key == "ma_method"){
            params[key] = cryptonite::randint(0, 4);
        }
        else {
            params[key] = sample_gaussian_int(val, sample_std,2, val + sample_offset);

        }
    }
    return params;
}

IndicatorConfig Indicator::generate_config(double exploration_prob) {
    IndicatorConfig config{&triggers[cryptonite::randint(triggers.size())],
                           get_random_params(exploration_prob),
                           this};
    cout << "The number of members in trigger " << endl;
    config.print();
    return config;
}

void Indicator::permute_level(int num_bars, IndicatorConfig &config, unordered_map<string, spda_t> &indicator_output) {
    if(config.trigger->has_level()){
        spda_t output{indicator_output[config.trigger->getComparand()]};
        double output_min{dMax}, output_max{dMin};
        for(int i=0; i < num_bars; i++){
            if(output[i] != output[i]){
                continue;  // Pass over NaNs;
            }
            output_min = std::min(output_min, output[i]);
            output_max = std::max(output_max, output[i]);
        }

        config.params["level"] = cryptonite::rand(output_min, output_max);
    }
}

spda_t Indicator::get_source(const Dataset &dataset, ApplyTo apply_to) {
    switch(apply_to){
        case ApplyTo::OPEN:
            return dataset.open;
        case ApplyTo::HIGH:
            return dataset.high;
        case ApplyTo::LOW:
            return dataset.low;
        case ApplyTo::CLOSE:
            return dataset.close;
        case ApplyTo::MEDIAN:
            return dataset.median;
        case ApplyTo::TYPICAL:
            return dataset.typical;
        case ApplyTo::WEIGHTED:
            return dataset.weighted;
        default:
            throw std::runtime_error("Undefined `apply_to` value!");
    }
}

spda_t Indicator::apply_ma(int num_bars, double period, spda_t source, MAMethod ma_method) {
    switch (ma_method) {
        case MAMethod::SMA:
            return CIndicator::sma(num_bars, {source}, {period})[0];
        case MAMethod::WMA:
            return CIndicator::wma(num_bars, {source}, {period})[0];
        case MAMethod::EMA:
            return CIndicator::ema(num_bars, {source}, {period})[0];
        case MAMethod::SSMA:
            return CIndicator::wilders(num_bars, {source}, {period})[0];
        default:
            throw std::runtime_error("Unrecognized `ma_method`!");
    };
}

spda_t shift(int num_bars, int offset, spda_t source, double fill_value) {
    spda_t result{new double[num_bars]};
    int split{};
    if(offset > 0){
        split = offset;
        for(int i=0; i<split; i++){
            result[i] = fill_value;
        }
        for(int i=split; i<num_bars; i++){
            result[i] = source[i-split];
        }
    } else{
        split = num_bars - offset;
        for(int i=0; i<split; i++){
            result[i] = source[i+split];
        }
        for(int i=split; i<num_bars; i++){
            result[i] = fill_value;
        }
    }
    return result;
}

spda_t max_arrays(int num_bars, spda_t a, spda_t b) {
    spda_t result{new double[num_bars]};
    for(int i=0; i<num_bars; i++){
        result[i] = std::max(a[i], b[i]);
    }
    return result;
}

spda_t min_arrays(int num_bars, spda_t a, spda_t b) {
    spda_t result{new double[num_bars]};
    for(int i=0; i<num_bars; i++){
        result[i] = std::min(a[i], b[i]);
    }
    return result;
}

void IndicatorConfig::print() {
    trigger->to_json();
    for (auto const& [key, val] : params){
        cout << key << ": " << val << endl;
    }
    indicator->to_json();
}

unordered_map<string, spda_t> AcceleratorOscillator::compute(const Dataset &dataset, const IndicatorConfig &config) {
//        auto a = sma(dataset.num_bars, {dataset.median}, {5.0})[0];
//        auto b = sma(dataset.num_bars, {dataset.median}, {34.0})[0];

//        check_sma(dataset.num_bars, a, dataset.median, 4, 5);
//        check_sma(dataset.num_bars, b, dataset.median, 33, 34);

    spda_t ao = sub(dataset.num_bars, {sma(dataset.num_bars, {dataset.median}, {5.0})[0], sma(dataset.num_bars, {dataset.median}, {34.0})[0]})[0];
//        check_sub(dataset.num_bars, ao, a, b, 33);
//        auto c = sma(dataset.num_bars, {ao}, {5.0})[0];
//        check_sma(dataset.num_bars, c, ao, 37, 5);
    spda_t ac = sub(dataset.num_bars, {ao, sma(dataset.num_bars, {ao}, {5.0})[0]})[0];
//        check_sub(dataset.num_bars, ac, ao, c, 37);
    unordered_map<string, spda_t> result{{"value", ac}};
    return result;
}

unordered_map<string, spda_t> AccumulationDistribution::compute(const Dataset &dataset, const IndicatorConfig &config) {
    unordered_map<string, spda_t> result{{"value", ad(dataset.num_bars, {dataset.high, dataset.low, dataset.close, dataset.volume})[0]}};
    return result;
}

unordered_map<string, spda_t> ADX::compute(const Dataset &dataset, const IndicatorConfig &config) {
    unordered_map<string, spda_t> result{{"value", ad(dataset.num_bars, {dataset.high, dataset.low, dataset.close}, {config.params.at("period")})[0]}};
    return result;
}

unordered_map<string, spda_t> Alligator::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.num_bars;
    unordered_map<string, spda_t> result;
    auto ma_method = (MAMethod) (int) config.params.at("ma_method");
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    auto jaws_shift = (int) config.params.at("jaws_shift");
    auto jaws_period = (int) config.params.at("jaws_period");
    result["jaws"] = shift(num_bars, jaws_shift, apply_ma(num_bars, jaws_period, source, ma_method));
    auto teeth_shift = (int) config.params.at("teeth_shift");
    auto teeth_period = (int) config.params.at("teeth_period");
    result["teeth"] = shift(num_bars, teeth_shift, apply_ma(num_bars, teeth_period, source, ma_method));
    auto lips_shift = (int) config.params.at("lips_shift");
    auto lips_period = (int) config.params.at("lips_period");
    result["lips"] = shift(num_bars, lips_shift, apply_ma(num_bars, lips_period, source, ma_method));
    return result;
}

unordered_map<string, spda_t> AverageTrueRange::compute(const Dataset &dataset, const IndicatorConfig &config) {
    unordered_map<string, spda_t> result{{"value", atr(dataset.num_bars, {dataset.high, dataset.low, dataset.close}, {config.params.at("period")})[0]}};
    return unordered_map<string, spda_t>();
}

unordered_map<string, spda_t> BearsPower::compute(const Dataset &dataset, const IndicatorConfig &config) {
    auto ema = atr(dataset.num_bars, {dataset.close}, {config.params.at("period")})[0];
    unordered_map<string, spda_t> result{{"value", sub(dataset.num_bars, {dataset.low, ema})[0]}};
    return result;
}

unordered_map<string, spda_t> BullsPower::compute(const Dataset &dataset, const IndicatorConfig &config) {
    auto ema = atr(dataset.num_bars, {dataset.close}, {config.params.at("period")})[0];
    unordered_map<string, spda_t> result{{"value", sub(dataset.num_bars, {dataset.high, ema})[0]}};
    return result;
}

unordered_map<string, spda_t> BollingerBands::compute(const Dataset &dataset, const IndicatorConfig &config) {
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    auto bbs = bbands(dataset.num_bars, {source}, {config.params.at("period"),
                                                   config.params.at("deviation")});
    unordered_map<string, spda_t> result{{"bar", dataset.open}, {"lower", bbs[0]}, {"upper", bbs[2]}};
    return result;
}

unordered_map<string, spda_t> CommodityChannelIndex::compute(const Dataset &dataset, const IndicatorConfig &config) {
    unordered_map<string, spda_t> result{{"value", cci(dataset.num_bars, {dataset.high, dataset.low, dataset.close}, {config.params.at("period")})[0]}};
    return result;
}

unordered_map<string, spda_t> DeMarker::compute(const Dataset &dataset, const IndicatorConfig &config) {
    // refer: https://www.metatrader5.com/en/terminal/help/indicators/oscillators/demarker
    int num_bars = dataset.num_bars;
    auto high_i_1 = shift(num_bars, 1, dataset.high);  //  i-1
    auto low_i_1 = shift(num_bars, 1, dataset.low);  // i-1
    auto zero = sub(num_bars, {dataset.high, dataset.high})[0];
    auto diff_high =  sub(num_bars, {dataset.high, high_i_1})[0];
    auto de_max = max_arrays(num_bars, diff_high, zero);
    auto diff_low = sub(num_bars, {low_i_1, dataset.low})[0];
    auto de_min = max_arrays(num_bars, diff_low, zero);
    auto numerator = sma(num_bars, {de_max}, {config.params.at("period")})[0];
    auto denominator = sma(num_bars, {de_min}, {config.params.at("period")})[0];
    auto value = CIndicator::div(num_bars, {numerator, add(num_bars, {denominator, numerator})[0]})[0];
    unordered_map<string, spda_t> result{{"value", value}};
    return result;
}

unordered_map<string, spda_t> DirectionalIndicators::compute(const Dataset &dataset, const IndicatorConfig &config) {
    unordered_map<string, spda_t> result{{"value", di(dataset.num_bars, {dataset.high, dataset.low, dataset.close}, {config.params.at("period")})[0]}};
    return result;
}

unordered_map<string, spda_t> DonchianChannel::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.num_bars;
    unordered_map<string, spda_t> result{};
    result["bar"] = dataset.open;
    result["lower"] = CIndicator::min(num_bars, {dataset.low}, {config.params.at("period")})[0];
    result["upper"] = CIndicator::max(num_bars, {dataset.high}, {config.params.at("period")})[0];
    return result;
}

unordered_map<string, spda_t> Envelopes::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.num_bars;
    auto ma_method = (MAMethod) (int) config.params.at("ma_method");
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    auto sma = apply_ma(num_bars, (int) config.params.at("period"), source, ma_method);
    spda_t upper{new double[num_bars]}, lower{new double [num_bars]};
    double dev_pct = config.params.at("deviation_pct") / 100.0;
    for(int i=0; i< num_bars; i++){
        upper[i] = sma[i] * (1.0 + dev_pct);
        lower[i] = sma[i] * (1.0 - dev_pct);
    }

    unordered_map<string, spda_t> result{{"bar", dataset.open}, {"upper", upper}, {"lower", lower}};
    return result;
}

unordered_map<string, spda_t> ForceIndex::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.num_bars;
    auto ma_method = (MAMethod) (int) config.params.at("ma_method");
    auto zero = sub(num_bars, {dataset.high, dataset.high})[0];
    auto ma = apply_ma(num_bars, (int) config.params.at("period"), dataset.close, ma_method);
    auto ma_i_1 = shift(num_bars, 1, ma);  //  i-1
    spda_t fi{new double [num_bars]};
    auto volume = dataset.volume;
    for(int i=0; i < num_bars; i++){
        fi[i] = volume[i] * (ma[i] - ma_i_1[i]);
    }

    unordered_map<string, spda_t> result{{"value", fi}, {"zero", zero}};
    return result;
}

unordered_map<string, spda_t> MACD::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.num_bars;
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    auto fast_period = config.params.at("fast_period");
    auto slow_period = config.params.at("slow_period");
    auto signal_period = config.params.at("signal_period");
    auto zero = sub(num_bars, {dataset.high, dataset.high})[0];
    auto macd_ind = macd(num_bars, {source}, {fast_period, slow_period, signal_period})[0];
    unordered_map<string, spda_t> result{{"value", macd_ind}, {"zero", zero}};
    return result;
}

unordered_map<string, spda_t> MACDSignal::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.num_bars;
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    auto fast_period = config.params.at("fast_period");
    auto slow_period = config.params.at("slow_period");
    auto signal_period = config.params.at("signal_period");
    auto macd_ind = macd(num_bars, {source}, {fast_period, slow_period, signal_period});
    unordered_map<string, spda_t> result{{"macd", macd_ind[0]}, {"signal", macd_ind[1]}};
    return result;
}

unordered_map<string, spda_t> Momentum::compute(const Dataset &dataset, const IndicatorConfig &config) {
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    unordered_map<string, spda_t> result{{"value", mom(dataset.num_bars, {source}, {config.params.at("period")})[0]}};
    return result;
}

unordered_map<string, spda_t> MoneyFlowIndex::compute(const Dataset &dataset, const IndicatorConfig &config) {
    unordered_map<string, spda_t> result{{"value", mfi(dataset.num_bars, {dataset.high, dataset.low, dataset.close, dataset.volume}, {config.params.at("period")})[0]}};
    return result;
}

unordered_map<string, spda_t> MovingAverage::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.num_bars;
    auto ma_method = (MAMethod) (int) config.params.at("ma_method");
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    auto ma = apply_ma(num_bars, (int) config.params.at("period"), source, ma_method);
    int shift_val = (int) config.params.at("shift");
    unordered_map<string, spda_t> result{{"value", shift(num_bars, shift_val, ma)}, {"bar", dataset.open}};
    return result;
}

unordered_map<string, spda_t> MovingAverageOscillator::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.num_bars;
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    auto fast_period = config.params.at("fast_period");
    auto slow_period = config.params.at("slow_period");
    auto signal_period = config.params.at("signal_period");
    auto macd_ind = macd(num_bars, {source}, {fast_period, slow_period, signal_period});
    auto osma = sub(num_bars, {macd_ind[0], macd_ind[1]})[0];
    unordered_map<string, spda_t> result{{"value", osma}};
    return result;
}

unordered_map<string, spda_t> MovingAverageCrossOver::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.num_bars;
    auto ma_method = (MAMethod) (int) config.params.at("ma_method");
    unordered_map<string, spda_t> result{};
    result["fast"] = apply_ma(num_bars, (int) config.params.at("fast_period"), dataset.close, ma_method);
    result["slow"] = apply_ma(num_bars, (int) config.params.at("slow_period"), dataset.close, ma_method);
    return result;
}

unordered_map<string, spda_t> OnBalanceVolume::compute(const Dataset &dataset, const IndicatorConfig &config) {
    unordered_map<string, spda_t> result{{"value", obv(dataset.num_bars, {dataset.close, dataset.volume})[0]}};
    return result;
}
