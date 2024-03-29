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


std::unordered_map<std::string, double> Indicator::get_random_params(double exploration_prob) {
    std::unordered_map<std::string, double> params;
    for (auto const& [key, val] : defaults)
    {
        if (cryptonite::rand() > exploration_prob){ // level or don't explore
            params[key] = val;
        } else if (key == "apply_to"){
            params[key] = cryptonite::randint(0, 7);
        } else if (key == "ma_method"){
            params[key] = cryptonite::randint(0, 4);
        } else if(key == "level"){
//            assert(not std::isnan(min_level));
            params["level"] = cryptonite::rand(min_level, max_level);
        } else if( key.find("shift") != std::string::npos) {
            params[key] = sample_gaussian_int(val, sample_std, 0, val + sample_offset);
        } else if(key.find("pct") != std::string::npos){
            params[key] = sample_gaussian_int(val, sample_std, 1, val + sample_offset);
        } else { // period
            params[key] = sample_gaussian_int(val, sample_std,2, val + sample_offset);
        }
    }
    return params;
}

IndicatorConfig Indicator::generate_config(double exploration_prob) {
    IndicatorConfig config{triggers[cryptonite::randint(triggers.size())],
                           get_random_params(exploration_prob),
                           this};

    while( not validate_config(config)){
        config.params = get_random_params(exploration_prob);
    }
    config.toJson();
    return config;
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

bool Indicator::validate_config(IndicatorConfig &config) {
    return true;
}

void Indicator::set_level_range(double min_level, double max_level) {
    this->min_level = min_level;
    this->max_level = max_level;
}

spda_t shift(int num_bars, int offset, spda_t source, double fill_value) {

    spda_t result{new double[num_bars]};
    int split{};
    if(offset >= 0){
        split = offset;
        for(int i=0; i<split; i++){
            result[i] = fill_value;
        }
        for(int i=split; i<num_bars; i++){
            result[i] = source[i-split];
        }
    } else{
        split = num_bars + offset; // notice offset < 0
        for(int i=0; i<split; i++){
            result[i] = source[i-offset];  // notice offset < 0
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

spda_t rolling_sum(int num_bars, spda_t source, int period) {
    spda_t cumsum{new double [num_bars]}, result{new double[num_bars]};
    double cumsum_at_i = 0.0;

    for(int i=0; i< num_bars; i++){
        cumsum[i] = source[i] + cumsum_at_i;
        cumsum_at_i += source[i];
        if(i < period){
            result[i] = dNaN;
        } else {
            result[i] = cumsum[i] - cumsum[i-period];
        }
    }
    return result;
}

json IndicatorConfig::toJson() const {
    json j;
    j["indicator"] = indicator->get_name();
    j["trigger"] = trigger->toJson();
    for (auto const& [key, val] : params){
        j["params"][key] = val;
    }
    return j;

}

IndicatorConfig IndicatorConfig::fromJson(json j) {
    std::unordered_map<std::string, double> params{};
    auto indicator = Name2Indicator[j["indicator"].get<std::string>()];
    auto trigger = Trigger::fromJson(j["trigger"]);
    for(const auto& [k, v]: j["params"].items()){
        params[k] = v.get<double>();
    }

    return IndicatorConfig(trigger, params, indicator.get());
}


std::shared_ptr<bool[]> IndicatorConfig::compute(const Dataset &dataset, bool contra_trigger) {
    int num_bars = dataset.numBars;
    std::unordered_map<std::string, std::shared_ptr<double[]>> output = this->indicator->compute(dataset, *this);
    std::shared_ptr<Trigger> trigger = contra_trigger ?  this->trigger->get_contra() : this-> trigger;
    std::shared_ptr<bool[]> result{nullptr};

    std::string comparator = trigger->getComparator();
    if (comparator == ""){
        result = trigger->compute(num_bars, output[trigger->getComparand()]);
    } else if (comparator == "level"){
        result = trigger->compute(num_bars, output[trigger->getComparand()], params["level"]);
    } else {
        result = trigger->compute(num_bars, output[trigger->getComparand()], output[comparator]);
    }
    return result;
}

std::unordered_map<std::string, spda_t> AcceleratorOscillator::compute(const Dataset &dataset, const IndicatorConfig &config) {
    spda_t ao = sub(dataset.numBars, {sma(dataset.numBars, {dataset.median}, {5.0})[0], sma(dataset.numBars, {dataset.median}, {34.0})[0]})[0];
    spda_t ac = sub(dataset.numBars, {ao, sma(dataset.numBars, {ao}, {5.0})[0]})[0];
    std::unordered_map<std::string, spda_t> result{{"value", ac}};
    return result;
}

std::unordered_map<std::string, spda_t> AccumulationDistribution::compute(const Dataset &dataset, const IndicatorConfig &config) {
    std::unordered_map<std::string, spda_t> result{{"value", ad(dataset.numBars, {dataset.high, dataset.low, dataset.close, dataset.volume})[0]}};
    return result;
}

std::unordered_map<std::string, spda_t> ADX::compute(const Dataset &dataset, const IndicatorConfig &config) {
    std::unordered_map<std::string, spda_t> result{{"value", adx(dataset.numBars, {dataset.high, dataset.low, dataset.close}, {config.params.at("period")})[0]}};
    return result;
}

std::unordered_map<std::string, spda_t> Alligator::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.numBars;
    std::unordered_map<std::string, spda_t> result;
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

std::unordered_map<std::string, spda_t> AverageTrueRange::compute(const Dataset &dataset, const IndicatorConfig &config) {
    std::unordered_map<std::string, spda_t> result{{"value", atr(dataset.numBars, {dataset.high, dataset.low, dataset.close}, {config.params.at("period")})[0]}};
    return result;
}

std::unordered_map<std::string, spda_t> BearsPower::compute(const Dataset &dataset, const IndicatorConfig &config) {
    auto ema_val = ema(dataset.numBars, {dataset.close}, {config.params.at("period")})[0];
    std::unordered_map<std::string, spda_t> result{{"value", sub(dataset.numBars, {dataset.low, ema_val})[0]}};
    return result;
}

std::unordered_map<std::string, spda_t> BullsPower::compute(const Dataset &dataset, const IndicatorConfig &config) {
    auto ema_val = ema(dataset.numBars, {dataset.close}, {config.params.at("period")})[0];
    std::unordered_map<std::string, spda_t> result{{"value", sub(dataset.numBars, {dataset.high, ema_val})[0]}};
    return result;
}

std::unordered_map<std::string, spda_t> BollingerBands::compute(const Dataset &dataset, const IndicatorConfig &config) {
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    auto bbs = bbands(dataset.numBars, {source}, {config.params.at("period"),
                                                  config.params.at("deviation")});
    std::unordered_map<std::string, spda_t> result{{"bar", dataset.open}, {"lower", bbs[0]}, {"upper", bbs[2]}};
    return result;
}

std::unordered_map<std::string, spda_t> CommodityChannelIndex::compute(const Dataset &dataset, const IndicatorConfig &config) {
    std::unordered_map<std::string, spda_t> result{{"value", cci(dataset.numBars, {dataset.high, dataset.low, dataset.close}, {config.params.at("period")})[0]}};
    return result;
}

std::unordered_map<std::string, spda_t> DeMarker::compute(const Dataset &dataset, const IndicatorConfig &config) {
    // refer: https://www.metatrader5.com/en/terminal/help/indicators/oscillators/demarker
    int num_bars = dataset.numBars;
    auto high_i_1 = shift(num_bars, 1, dataset.high);  //  i-1
    auto low_i_1 = shift(num_bars, 1, dataset.low);  // i-1
    auto diff_high =  sub(num_bars, {dataset.high, high_i_1})[0];
    auto de_max = max_arrays(num_bars, diff_high, dataset.zero);
    auto diff_low = sub(num_bars, {low_i_1, dataset.low})[0];
    auto de_min = max_arrays(num_bars, diff_low, dataset.zero);
    auto numerator = sma(num_bars, {de_max}, {config.params.at("period")})[0];
    auto denominator = sma(num_bars, {de_min}, {config.params.at("period")})[0];
    auto value = CIndicator::div(num_bars, {numerator, add(num_bars, {denominator, numerator})[0]})[0];
    std::unordered_map<std::string, spda_t> result{{"value", value}};
    return result;
}

std::unordered_map<std::string, spda_t> DirectionalIndicators::compute(const Dataset &dataset, const IndicatorConfig &config) {
    auto value = di(dataset.numBars, {dataset.high, dataset.low, dataset.close}, {config.params.at("period")});
    std::unordered_map<std::string, spda_t> result{{"plus", value[0]}, {"minus", value[1]}};
    return result;
}

std::unordered_map<std::string, spda_t> DonchianChannel::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.numBars;
    std::unordered_map<std::string, spda_t> result{};
    result["bar"] = dataset.open;
    result["lower"] = CIndicator::min(num_bars, {dataset.low}, {config.params.at("period")})[0];
    result["upper"] = CIndicator::max(num_bars, {dataset.high}, {config.params.at("period")})[0];
    return result;
}

std::unordered_map<std::string, spda_t> Envelopes::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.numBars;
    auto ma_method = (MAMethod) (int) config.params.at("ma_method");
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    auto sma = apply_ma(num_bars, (int) config.params.at("period"), source, ma_method);
    spda_t upper{new double[num_bars]}, lower{new double [num_bars]};
    double dev_pct = config.params.at("deviation_pct") / 100.0;
    for(int i=0; i< num_bars; i++){
        upper[i] = sma[i] * (1.0 + dev_pct);
        lower[i] = sma[i] * (1.0 - dev_pct);
    }

    std::unordered_map<std::string, spda_t> result{{"bar", dataset.open}, {"upper", upper}, {"lower", lower}};
    return result;
}

std::unordered_map<std::string, spda_t> ForceIndex::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.numBars;
    auto ma_method = (MAMethod) (int) config.params.at("ma_method");
    auto ma = apply_ma(num_bars, (int) config.params.at("period"), dataset.close, ma_method);
    auto ma_i_1 = shift(num_bars, 1, ma);  //  i-1
    spda_t fi{new double [num_bars]};
    auto volume = dataset.volume;
    for(int i=0; i < num_bars; i++){
        fi[i] = volume[i] * (ma[i] - ma_i_1[i]);
    }

    std::unordered_map<std::string, spda_t> result{{"value", fi}, {"zero", dataset.zero}};
    return result;
}

std::unordered_map<std::string, spda_t> MACD::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.numBars;
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    auto fast_period = config.params.at("fast_period");
    auto slow_period = config.params.at("slow_period");
    auto signal_period = config.params.at("signal_period");
    auto macd_ind = macd(num_bars, {source}, {fast_period, slow_period, signal_period})[0];
    std::unordered_map<std::string, spda_t> result{{"value", macd_ind}, {"zero", dataset.zero}};
    return result;
}

bool MACD::validate_config(IndicatorConfig &config) {
    return config.params["fast_period"] < config.params["slow_period"];
}

std::unordered_map<std::string, spda_t> MACDSignal::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.numBars;
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    auto fast_period = config.params.at("fast_period");
    auto slow_period = config.params.at("slow_period");
    auto signal_period = config.params.at("signal_period");
    auto macd_ind = macd(num_bars, {source}, {fast_period, slow_period, signal_period});
    std::unordered_map<std::string, spda_t> result{{"macd", macd_ind[0]}, {"signal", macd_ind[1]}};
    return result;
}

bool MACDSignal::validate_config(IndicatorConfig &config) {
    return config.params["fast_period"] < config.params["slow_period"];
}

std::unordered_map<std::string, spda_t> Momentum::compute(const Dataset &dataset, const IndicatorConfig &config) {
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    std::unordered_map<std::string, spda_t> result{{"value", mom(dataset.numBars, {source}, {config.params.at("period")})[0]}};
    return result;
}

std::unordered_map<std::string, spda_t> MoneyFlowIndex::compute(const Dataset &dataset, const IndicatorConfig &config) {
    std::unordered_map<std::string, spda_t> result{{"value", mfi(dataset.numBars, {dataset.high, dataset.low, dataset.close, dataset.volume}, {config.params.at("period")})[0]}};
    return result;
}

std::unordered_map<std::string, spda_t> MovingAverage::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.numBars;
    auto ma_method = (MAMethod) (int) config.params.at("ma_method");
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    auto ma = apply_ma(num_bars, (int) config.params.at("period"), source, ma_method);
    int shift_val = (int) config.params.at("shift");
    std::unordered_map<std::string, spda_t> result{{"value", shift(num_bars, shift_val, ma)}, {"bar", dataset.open}};
    return result;
}

std::unordered_map<std::string, spda_t> MovingAverageOscillator::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.numBars;
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    auto fast_period = config.params.at("fast_period");
    auto slow_period = config.params.at("slow_period");
    auto signal_period = config.params.at("signal_period");
    auto macd_ind = macd(num_bars, {source}, {fast_period, slow_period, signal_period});
    auto osma = sub(num_bars, {macd_ind[0], macd_ind[1]})[0];
    std::unordered_map<std::string, spda_t> result{{"value", osma}};
    return result;
}

bool MovingAverageOscillator::validate_config(IndicatorConfig &config) {
    return config.params["fast_period"] < config.params["slow_period"];
}

std::unordered_map<std::string, spda_t> MovingAverageCrossOver::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.numBars;
    auto ma_method = (MAMethod) (int) config.params.at("ma_method");
    std::unordered_map<std::string, spda_t> result{};
    result["fast"] = apply_ma(num_bars, (int) config.params.at("fast_period"), dataset.close, ma_method);
    result["slow"] = apply_ma(num_bars, (int) config.params.at("slow_period"), dataset.close, ma_method);
    return result;
}

std::unordered_map<std::string, spda_t> OnBalanceVolume::compute(const Dataset &dataset, const IndicatorConfig &config) {
    std::unordered_map<std::string, spda_t> result{{"value", obv(dataset.numBars, {dataset.close, dataset.volume})[0]}};
    return result;
}

std::unordered_map<std::string, spda_t> RelativeStrengthIndex::compute(const Dataset &dataset, const IndicatorConfig &config) {
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    std::unordered_map<std::string, spda_t> result{{"value", rsi(dataset.numBars, {source}, {config.params.at("period")})[0]}};
    return result;
}

std::unordered_map<std::string, spda_t> RelativeVigorIndex::compute(const Dataset &dataset, const IndicatorConfig &config) {
    // Ref: https://www.investopedia.com/terms/r/relative_vigor_index.asp
    int num_bars = dataset.numBars;

    auto a = sub(num_bars, {dataset.close, dataset.open})[0];
    auto b = shift(num_bars, 1, a);
    auto c = shift(num_bars, 1, b);
    auto d = shift(num_bars, 1, c);

    auto e = sub(num_bars, {dataset.high, dataset.low})[0];

    auto f = shift(num_bars, 1, e);
    auto g = shift(num_bars, 1, f);
    auto h = shift(num_bars, 1, g);

    spda_t num{new double [num_bars]}, dem{new double [num_bars]};

    for(int i=0; i< num_bars; i++){
        num[i] = (a[i] + 2.0 * b[i] + 2.0 * c[i] + d[i]) / 6.0;
        dem[i] = (e[i] + 2.0 * f[i] + 2.0 * g[i] + h[i]) / 6.0;
    }

    auto period = config.params.at("period");
    auto sma_num = sma(num_bars, {num}, {period})[0];
    auto sma_dem = sma(num_bars, {dem}, {period})[0];

    auto rvi = CIndicator::div(num_bars, {sma_num, sma_dem})[0];
    std::unordered_map<std::string, spda_t> result{{"value", rvi}};
    return result;
}

std::unordered_map<std::string, spda_t> RelativeVigorIndexSignal::compute(const Dataset &dataset, const IndicatorConfig &config) {
    // Ref: https://www.investopedia.com/terms/r/relative_vigor_index.asp
    int num_bars = dataset.numBars;

    auto a = sub(num_bars, {dataset.close, dataset.open})[0];
    auto b = shift(num_bars, 1, a);
    auto c = shift(num_bars, 1, b);
    auto d = shift(num_bars, 1, c);

    auto e = sub(num_bars, {dataset.high, dataset.low})[0];

    auto f = shift(num_bars, 1, e);
    auto g = shift(num_bars, 1, f);
    auto h = shift(num_bars, 1, g);

    spda_t num{new double [num_bars]}, dem{new double [num_bars]};

    for(int i=0; i< num_bars; i++){
        num[i] = (a[i] + 2.0 * b[i] + 2.0 * c[i] + d[i]) / 6.0;
        dem[i] = (e[i] + 2.0 * f[i] + 2.0 * g[i] + h[i]) / 6.0;
    }

    auto period = config.params.at("period");
    auto sma_num = sma(num_bars, {num}, {period})[0];
    auto sma_dem = sma(num_bars, {dem}, {period})[0];

    auto rvi = CIndicator::div(num_bars, {sma_num, sma_dem})[0];
    auto rvi_1 = shift(num_bars, 1, rvi);
    auto rvi_2 = shift(num_bars, 1, rvi_1);
    auto rvi_3 = shift(num_bars, 1, rvi_2);
    spda_t signal{new double [num_bars]};
    for(int i=0; i< num_bars; i++){
        signal[i] =(rvi[i] + 2.0 * rvi_1[i] + 2.0 * rvi_2[i] + rvi_3[i]) / 6.0;
    }

    std::unordered_map<std::string, spda_t> result{{"rvi", rvi}, {"signal", signal}};
    return result;
}

std::unordered_map<std::string, spda_t> StandardDeviation::compute(const Dataset &dataset, const IndicatorConfig &config) {
    auto source = get_source(dataset, (ApplyTo) (int) config.params.at("apply_to"));
    std::unordered_map<std::string, spda_t> result{{"value", stddev(dataset.numBars, {source}, {config.params.at("period")})[0]}};
    return result;
}

std::unordered_map<std::string, spda_t> Stochastic::compute(const Dataset &dataset, const IndicatorConfig &config) {
    auto pct_k_period = config.params.at("pct_k_period");
    auto pct_k_slowing_period = config.params.at("pct_k_slowing_period");
    auto pct_d_period = config.params.at("pct_d_period");
    auto stoch_ind = stoch(dataset.numBars,
                           {dataset.high, dataset.low, dataset.close},
                           {pct_k_period, pct_k_slowing_period, pct_d_period});

    std::unordered_map<std::string, spda_t> result{{"value",stoch_ind[0]}};
    return result;
}

std::unordered_map<std::string, spda_t> StochasticSignal::compute(const Dataset &dataset, const IndicatorConfig &config) {
    auto pct_k_period = config.params.at("pct_k_period");
    auto pct_k_slowing_period = config.params.at("pct_k_slowing_period");
    auto pct_d_period = config.params.at("pct_d_period");
    auto stoch_ind = stoch(dataset.numBars,
                           {dataset.high, dataset.low, dataset.close},
                           {pct_k_period, pct_k_slowing_period, pct_d_period});

    std::unordered_map<std::string, spda_t> result{{"stoch", stoch_ind[0]}, {"signal", stoch_ind[1]}};
    return result;
}

std::unordered_map<std::string, spda_t> Volumes::compute(const Dataset &dataset, const IndicatorConfig &config) {
    return std::unordered_map<std::string, spda_t>{{"value", dataset.volume}};
}

std::unordered_map<std::string, spda_t> WilliamsPercentRange::compute(const Dataset &dataset, const IndicatorConfig &config) {
    std::unordered_map<std::string, spda_t> result{{"value", willr(dataset.numBars, {dataset.high, dataset.low, dataset.close}, {config.params.at("period")})[0]}};
    return result;
}

std::unordered_map<std::string, spda_t> CandleColor::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.numBars;
    double min_change_pct = config.params.at("min_change_pct") / 100.0;
    spda_t offset{new double[num_bars]}, bearish{new double[num_bars]}, bullish{new double [num_bars]};
    for(int i=0; i<num_bars; i++){
        offset[i] = dataset.open[i] * min_change_pct;
        bearish[i] = 0.0;
        bullish[i] = 0.0;

        if (dataset.close[i] < (dataset.open[i] - offset[i])){
            bearish[i] = 1.0;
        } else if (dataset.close[i] > (dataset.open[i] + offset[i])){
            bullish[i] = 1.0;
        }
    }

    auto period = (int) config.params.at("consecutive_period");

    auto rolling_bear = rolling_sum(num_bars, bearish, period);
    auto rolling_bull = rolling_sum(num_bars, bullish, period);

    spda_t cc{new double[num_bars]};
    for(int i=0; i< num_bars; i++){
        if(rolling_bull[i] >= period){
            cc[i] = 1.0;
        } else if (rolling_bear[i] >= period){
            cc[i] = -1.0;
        } else {
            cc[i] = 0.0;
        }
    }

    std::unordered_map<std::string, spda_t> result {{"value", cc}, {"zero", dataset.zero}};
    return result;
}

std::unordered_map<std::string, spda_t> PinBar::compute(const Dataset &dataset, const IndicatorConfig &config) {
    int num_bars = dataset.numBars;
    spda_t pb{new double[num_bars]};
    double body_len{}, wick_len{}, nose_len{}, body_pct{}, wick_pct{}, tot_len{};
    double max_body_pct = config.params.at("max_body_pct");
    double min_wick_pct = config.params.at("min_wick_pct");
    for(int i=0; i<num_bars; i++){
        if (dataset.close[i] < dataset.open[i]){ // bearish pin
            wick_len = dataset.high[i] - dataset.open[i];
            nose_len = dataset.close[i] - dataset.low[i];
            body_len = dataset.open[i] - dataset.close[i];
        } else { // bullish pin
            wick_len = dataset.open[i] - dataset.low[i];
            nose_len = dataset.high[i] - dataset.close[i];
            body_len = dataset.close[i] - dataset.open[i];
        }
        tot_len = (dataset.high[i] - dataset.low[i]);
        body_pct = ( body_len * 100.0 ) / tot_len;
        wick_pct = ( wick_len * 100.0) / tot_len;
        pb[i] = 0.0;
        if (wick_len > body_len && nose_len < wick_len && body_pct <= max_body_pct && wick_pct >= min_wick_pct){
            if (dataset.close[i] < dataset.open[i]){ // bearish pin
                pb[i] = -1.0;
            } else if (dataset.close[i] > dataset.open[i]) { // bullish pin
               pb[i] = 1.0;
            }
        }
    }

    std::unordered_map<std::string, spda_t> result {{"value", pb}, {"zero", dataset.zero}};
    return result;
}


std::vector<std::shared_ptr<Indicator>> Indicators{
        std::make_shared<AcceleratorOscillator>(),
        std::make_shared<AccumulationDistribution>(),
        std::make_shared<ADX>(),
        std::make_shared<Alligator>(),
        std::make_shared<AverageTrueRange>(),
        std::make_shared<BearsPower>(),
        std::make_shared<BullsPower>(),
        std::make_shared<BollingerBands>(),
        std::make_shared<CommodityChannelIndex>(),
        std::make_shared<DeMarker>(),
        std::make_shared<DirectionalIndicators>(),
        std::make_shared<DonchianChannel>(),
        std::make_shared<Envelopes>(),
        std::make_shared<ForceIndex>(),
        std::make_shared<MACD>(),
        std::make_shared<MACDSignal>(),
        std::make_shared<Momentum>(),
        std::make_shared<MoneyFlowIndex>(),
        std::make_shared<MovingAverage>(),
        std::make_shared<MovingAverageOscillator>(),
        std::make_shared<MovingAverageCrossOver>(),
        std::make_shared<OnBalanceVolume>(),
        std::make_shared<RelativeStrengthIndex>(),
        std::make_shared<RelativeVigorIndex>(),
        std::make_shared<RelativeVigorIndexSignal>(),
        std::make_shared<StandardDeviation>(),
        std::make_shared<Stochastic>(),
        std::make_shared<StochasticSignal>(),
        std::make_shared<Volumes>(),
        std::make_shared<WilliamsPercentRange>(),
        std::make_shared<CandleColor>(),
        std::make_shared<PinBar>()
};

std::unordered_map<std::string,std::shared_ptr<Indicator>> _setName2Indicator(){
    std::unordered_map<std::string,std::shared_ptr<Indicator>> result{};
    for (auto& it: Indicators) {
        result[it->get_name()] = it;
    }
    return result;
}

std::unordered_map<std::string,std::shared_ptr<Indicator>> Name2Indicator = _setName2Indicator();


void Indicator::setup(const Dataset &dataset) {
    int num_bars = dataset.numBars;
    int num_indicators = Indicators.size();
    for(int i=0; i<num_indicators; i++){
        std::shared_ptr<Indicator> indicator = Indicators[i];
        IndicatorConfig config = indicator->generate_config(0.0);
        if(indicator->has_level()){
            while(not config.trigger->has_level()){
                config = indicator->generate_config(0.0);
            }
            auto indicator_output = indicator->compute(dataset, config);
            spda_t output{indicator_output[config.trigger->getComparand()]};
            double output_min{dMax}, output_max{dMin};
            for(int i=0; i < num_bars; i++){
                if(std::isnan(output[i])){
                    continue;  // Pass over NaNs;
                }
                output_min = std::min(output_min, output[i]);
                output_max = std::max(output_max, output[i]);
            }
            indicator->set_level_range(output_min, output_max);
        }
    }
}

std::string Indicator::get_name() {
    return name;
}

