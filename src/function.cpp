//
// Created by Abhishek Aggarwal on 29/10/2021.
//

#include "function.h"


namespace CIndicator{
    Function::Function(int idx) {
        info = ti_indicators + idx;
        num_params = info->options;
        num_inputs = info->inputs;
        num_outputs = info->outputs;
    }

    std::vector<output_type> Function::operator()(int input_len, const std::initializer_list<std::shared_ptr<double[]>> &inputs, std::initializer_list<double> params) {

        if (num_params > 0 && params.size() == 0 || num_params == 0 && params.size() > 0 || num_inputs != inputs.size()){
            throw std::invalid_argument("Params cannot be empty");
        }

        if (num_params == 0) {
            params = {0.0};
        }


        TI_REAL *c_inputs[TI_MAXINDPARAMS];
        TI_REAL *c_params = (TI_REAL*) data(params);
        int min_input_len = input_len, i = 0;
        for(auto input: inputs){
            int num_nan{0};
            for (int j = 0; j < input_len; j++) {
                if (std::isnan(input[j])) {
                    num_nan += 1;
                } else {
                    break;
                }
            }
            min_input_len = std::min(min_input_len, input_len - num_nan);
            i++;
        }
        int delta = info->start(c_params);
        i = 0;
        for(auto input: inputs){
            c_inputs[i] = &input[input_len - min_input_len];
            i++;
        }

        if (min_input_len - delta <= 0) {
            throw std::invalid_argument("The params dont allow `min_input_len`");
        }

        TI_REAL *c_outputs[TI_MAXINDPARAMS];
        std::vector<output_type> outputs{};
        for (int i = 0; i < num_outputs; i++) {
            outputs.push_back(std::shared_ptr<double[]>{new double[input_len]});
            for(int j=0; j < (input_len - min_input_len) + delta; j++){
                outputs[i][j] = dNaN;
            }
            c_outputs[i] = &outputs[i][(input_len - min_input_len) + delta];
        }

        int ret = info->indicator(min_input_len, c_inputs, c_params, c_outputs);
        if (ret != 0) {
            std::cout << "Name: " << info->full_name << std::endl;
            for(int i=0; i< num_params; i++){
                std::cout<< info->option_names[i] << ": " << c_params[i] <<std::endl;
            }
            throw std::invalid_argument("The params dont allow computation");
        }
        return outputs;
    }

    Function abs{0};
    Function acos{1};
    Function ad{2};
    Function add{3};
    Function adosc{4};
    Function adx{5};
    Function adxr{6};
    Function ao{7};
    Function apo{8};
    Function aroon{9};
    Function aroonosc{10};
    Function asin{11};
    Function atan{12};
    Function atr{13};
    Function avgprice{14};
    Function bbands{15};
    Function bop{16};
    Function cci{17};
    Function ceil{18};
    Function cmo{19};
    Function cos{20};
    Function cosh{21};
    Function crossany{22};
    Function crossover{23};
    Function cvi{24};
    Function decay{25};
    Function dema{26};
    Function di{27};
    Function div{28};
    Function dm{29};
    Function dpo{30};
    Function dx{31};
    Function edecay{32};
    Function ema{33};
    Function emv{34};
    Function exp{35};
    Function fisher{36};
    Function floor{37};
    Function fosc{38};
    Function hma{39};
    Function kama{40};
    Function kvo{41};
    Function lag{42};
    Function linreg{43};
    Function linregintercept{44};
    Function linregslope{45};
    Function ln{46};
    Function log10{47};
    Function macd{48};
    Function marketfi{49};
    Function mass{50};
    Function max{51};
    Function md{52};
    Function medprice{53};
    Function mfi{54};
    Function min{55};
    Function mom{56};
    Function msw{57};
    Function mul{58};
    Function natr{59};
    Function nvi{60};
    Function obv{61};
    Function ppo{62};
    Function psar{63};
    Function pvi{64};
    Function qstick{65};
    Function roc{66};
    Function rocr{67};
    Function round{68};
    Function rsi{69};
    Function sin{70};
    Function sinh{71};
    Function sma{72};
    Function sqrt{73};
    Function stddev{74};
    Function stderr{75};
    Function stoch{76};
    Function stochrsi{77};
    Function sub{78};
    Function sum{79};
    Function tan{80};
    Function tanh{81};
    Function tema{82};
    Function todeg{83};
    Function torad{84};
    Function tr{85};
    Function trima{86};
    Function trix{87};
    Function trunc{88};
    Function tsf{89};
    Function typprice{90};
    Function ultosc{91};
    Function var{92};
    Function vhf{93};
    Function vidya{94};
    Function volatility{95};
    Function vosc{96};
    Function vwma{97};
    Function wad{98};
    Function wcprice{99};
    Function wilders{100};
    Function willr{101};
    Function wma{102};
    Function zlema{103};
}
