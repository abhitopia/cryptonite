//
// Created by Abhishek Aggarwal on 29/10/2021.
//

#ifndef CRYPTONITE_FUNCTION_H
#define CRYPTONITE_FUNCTION_H
#include "../include/indicators/cindicators.h"
#include "constants.h"
#include <memory>
#include <vector>
#include <stdexcept>


using namespace std;

namespace CIndicator{
    typedef shared_ptr<double[]> output_type;

    struct Function {
        const ti_indicator_info *info{nullptr};
        int num_params{};
        int num_inputs{};
        int num_outputs{};

        Function(int idx);
        vector<output_type> operator()(int input_len, const initializer_list<shared_ptr<double[]>> &inputs,
                                       initializer_list<double> params= {});
    };

    extern Function abs;
    extern Function acos;
    extern Function ad;
    extern Function add;
    extern Function adosc;
    extern Function adx;
    extern Function adxr;
    extern Function ao;
    extern Function apo;
    extern Function aroon;
    extern Function aroonosc;
    extern Function asin;
    extern Function atan;
    extern Function atr;
    extern Function avgprice;
    extern Function bbands;
    extern Function bop;
    extern Function cci;
    extern Function ceil;
    extern Function cmo;
    extern Function cos;
    extern Function cosh;
    extern Function crossany;
    extern Function crossover;
    extern Function cvi;
    extern Function decay;
    extern Function dema;
    extern Function di;
    extern Function div;
    extern Function dm;
    extern Function dpo;
    extern Function dx;
    extern Function edecay;
    extern Function ema;
    extern Function emv;
    extern Function exp;
    extern Function fisher;
    extern Function floor;
    extern Function fosc;
    extern Function hma;
    extern Function kama;
    extern Function kvo;
    extern Function lag;
    extern Function linreg;
    extern Function linregintercept;
    extern Function linregslope;
    extern Function ln;
    extern Function log10;
    extern Function macd;
    extern Function marketfi;
    extern Function mass;
    extern Function max;
    extern Function md;
    extern Function medprice;
    extern Function mfi;
    extern Function min;
    extern Function mom;
    extern Function msw;
    extern Function mul;
    extern Function natr;
    extern Function nvi;
    extern Function obv;
    extern Function ppo;
    extern Function psar;
    extern Function pvi;
    extern Function qstick;
    extern Function roc;
    extern Function rocr;
    extern Function round;
    extern Function rsi;
    extern Function sin;
    extern Function sinh;
    extern Function sma;
    extern Function sqrt;
    extern Function stddev;
    extern Function stderr;
    extern Function stoch;
    extern Function stochrsi;
    extern Function sub;
    extern Function sum;
    extern Function tan;
    extern Function tanh;
    extern Function tema;
    extern Function todeg;
    extern Function torad;
    extern Function tr;
    extern Function trima;
    extern Function trix;
    extern Function trunc;
    extern Function tsf;
    extern Function typprice;
    extern Function ultosc;
    extern Function var;
    extern Function vhf;
    extern Function vidya;
    extern Function volatility;
    extern Function vosc;
    extern Function vwma;
    extern Function wad;
    extern Function wcprice;
    extern Function wilders;
    extern Function willr;
    extern Function wma;
    extern Function zlema;
}

#endif //CRYPTONITE_FUNCTION_H
