//
// Created by Abhishek Aggarwal on 26/12/2021.
//

#ifndef CRYPTONITE_BINANCE_H
#define CRYPTONITE_BINANCE_H
#include <iostream>
#include "../include/json.h"
#include <cpr/cpr.h>

using json = nlohmann::json;

static std::string BASE_URL{"https://api.binance.com"};

enum Interval {
    MINUTE1 = 0,
    MINUTE3 = 1,
    MINUTE5 = 2,
    MINUTE15 = 3,
    MINUTE30 = 4,
    HOUR1 = 5,
    HOUR2 = 6,
    HOUR4 = 7,
    HOUR6 = 8,
    HOUR8 = 9,
    HOUR12 = 10,
    DAY1 = 11,
    DAY3 = 12,
    WEEK1 = 13,
    MONTH1 = 14
};

Interval stringToInterval(std::string text);
std::string intervalToString(Interval interval);
int intervalToSeconds(Interval interval);

class BinanceAPI {
    std::string publicKey{""};
    std::string secretKey{""};

    json handleErrorOrReturnJson(cpr::Response r){
        if(r.status_code != 200){
            std::cerr << "Error [" << r.status_code << "] making request to [" << r.url << "]" << std::endl;
            std::cerr << r.text << std::endl;
            throw std::runtime_error("Exiting due to API error.");
        }
        json value = json::parse(r.text);
        return value;
    }

public:
    BinanceAPI(){};

    json getExchangeInfo(std::string symbol = ""){
        auto params = cpr::Parameters{};
        if(symbol.length() > 0){
            params.Add({"symbol", symbol});
        }
        std::string endPoint = "/api/v3/exchangeInfo";
        cpr::Response r = cpr::Get(cpr::Url{BASE_URL + endPoint}, params);
        return handleErrorOrReturnJson(r);
    }
    json getOHLCVdata(std::string symbol, Interval interval, long startFrom = -1){
        auto params = cpr::Parameters{};
        params.Add({"symbol", symbol});
        params.Add({"interval", intervalToString(interval)});
        params.Add({"limit", "1000"});

        if(startFrom <= 0){
            params.Add({"startTime", std::to_string(1576746000)}); //  Pretty much from the beginning, notice the missing * 1000
        } else {
            params.Add({"startTime", std::to_string(startFrom * 1000)}); //  Corresponds to 2019-06-19 09:00:00
        }

        std::string endPoint = "/api/v3/klines";
        cpr::Response r = cpr::Get(cpr::Url{BASE_URL + endPoint}, params);
        auto v = handleErrorOrReturnJson(r);
        return v;

    }

};


#endif //CRYPTONITE_BINANCE_H
