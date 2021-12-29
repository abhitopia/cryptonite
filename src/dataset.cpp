//
// Created by Abhishek Aggarwal on 25/10/2021.
//
#include <iostream>
#include "dataset.h"
#include "../include/csv.h"
#include "../include/date.h"


using namespace io;
using namespace date;
using namespace std::chrono;


Dataset Dataset::from_csv(std::string filename) {
    CSVReader<6>csv(filename.c_str());
    csv.read_header(io::ignore_no_column, "timestamp", "Open", "High", "Low", "Close", "Volume");

    std::string timestamp{};
    double open{}, high{}, low{}, close{}, volume{};
    int tp_seconds{}, num_rows{0};
    std::istringstream t_stream{};

    std::vector<int> timestamps{};
    std::vector<double> opens{};
    std::vector<double> highs{};
    std::vector<double> lows{};
    std::vector<double> closes{};
    std::vector<double> volumes{};
    date::sys_seconds tp;

    while(csv.read_row(timestamp, open, high, low, close, volume)){
        t_stream.str(timestamp.c_str());
        t_stream >> date::parse("%Y-%m-%d %H:%M:%S", tp);
        if(t_stream.fail()){
            throw std::runtime_error("Unable to parse timestamp!");
        }
        tp_seconds = (int) floor <seconds>(tp).time_since_epoch().count();
        timestamps.push_back(tp_seconds);
        opens.push_back(open);
        lows.push_back(low);
        closes.push_back(close);
        volumes.push_back(volume);
        num_rows += 1;
    }

    int* timestamp_ptr{new int[num_rows]};
    double* open_ptr{new double [num_rows]};
    double* low_ptr{new double [num_rows]};
    double* high_ptr{new double [num_rows]};
    double* close_ptr{new double [num_rows]};
    double* volume_ptr{new double [num_rows]};
    std::copy(timestamps.begin(), timestamps.end(), timestamp_ptr);
    std::copy(opens.begin(), opens.end(), open_ptr);
    std::copy(highs.begin(), highs.end(), high_ptr);
    std::copy(lows.begin(), lows.end(), low_ptr);
    std::copy(closes.begin(), closes.end(), close_ptr);
    std::copy(volumes.begin(), volumes.end(), volume_ptr);
    return Dataset(num_rows, timestamp_ptr, open_ptr, high_ptr, low_ptr, close_ptr, volume_ptr);
}

Dataset::Dataset(int num_bars, int* timestamp, double* open, double* high,
                 double* low, double* close, double* volume) {
    this->num_bars = num_bars;
    this->timestamp.reset(timestamp);
    this->open.reset(open);
    this->high.reset(high);
    this->low.reset(low);
    this->close.reset(close);
    this->volume.reset(volume);
    this->median = CIndicator::medprice(num_bars, {this->high, this->low})[0];
    this->typical = CIndicator::typprice(num_bars, {this->high, this->low, this->close})[0];
    this->weighted = CIndicator::wcprice(num_bars, {this->high, this->low, this->close})[0];
    this->zero = CIndicator::sub(num_bars, {this->high, this->high})[0];
}

