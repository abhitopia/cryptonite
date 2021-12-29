//
// Created by Abhishek Aggarwal on 26/12/2021.
//



#include "binance.h"

int intervalToSeconds(Interval interval) {
    switch(interval){
        case MINUTE1: return 60;
        case MINUTE3: return 3 * 60;
        case MINUTE5: return 5 * 60;
        case MINUTE15: return 15 * 60;
        case MINUTE30: return 30 * 60;
        case HOUR1: return 60 * 60;
        case HOUR2: return 2 * 60 * 60;
        case HOUR4: return 4 * 60 * 60;
        case HOUR6: return 6 * 60 * 60;
        case HOUR8: return 8 * 60 * 60;
        case HOUR12: return 12 * 60 * 60;
        case DAY1: return 24 * 60 * 60;
        case DAY3: return 3 * 24 * 60 * 60;
        case WEEK1: return 7 * 24 * 60 * 60;
        case MONTH1: return 30 * 24 * 60 * 60;
    }
}

std::string intervalToString(Interval interval) {
    switch(interval){
        case MINUTE1: return "1m";
        case MINUTE3: return "3m";
        case MINUTE5: return "5m";
        case MINUTE15: return "15m";
        case MINUTE30: return "30m";
        case HOUR1: return "1h";
        case HOUR2: return "2h";
        case HOUR4: return "4h";
        case HOUR6: return "6h";
        case HOUR8: return "8h";
        case HOUR12: return "12h";
        case DAY1: return "1d";
        case DAY3: return "3d";
        case WEEK1: return "1w";
        case MONTH1: return "1M";
    }
}
