//
// Created by Abhishek Aggarwal on 26/12/2021.
//

#ifndef CRYPTONITE_DATASTORE_H
#define CRYPTONITE_DATASTORE_H

#include <string>
#include <filesystem>
#include <iostream>
#include "binance.h"
#include "json_file_handler.h"


namespace fs = std::filesystem;

struct DataSetInfo {
    std::string baseAsset;
    std::string quoteAsset;
    Interval interval;

    DataSetInfo(std::string baseAsset, std::string quoteAsset, Interval interval){
        this->baseAsset = baseAsset;
        this->quoteAsset = quoteAsset;
        this->interval = interval;
        validate();
    }

    std::string symbol(){
        return baseAsset + quoteAsset;
    }

    int intervalInSeconds(){
        return intervalToSeconds(interval);
    }

    std::string intervalInString(){
        return intervalToString(interval);
    }

    void validate(){
        // check if symbol exists using Binance API and that trading is permitted.
    }
};

struct DataSetV2 {
    DataSetInfo info;
    std::shared_ptr<std::vector<long>> timestamp = std::make_shared<std::vector<long>>();
    std::shared_ptr<std::vector<double>> open = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> high = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> low = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> close = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> volume = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> median = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> typical = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> weighted = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> zero = std::make_shared<std::vector<double>>();

    DataSetV2(DataSetInfo info): info{info}{};

    void add(long timestamp, double open, double high, double low, double close, double volume){
        if(this->timestamp->size() > 0){
            while(this->timestamp->back() + info.intervalInSeconds() != timestamp){
//                std::cout << "Missing Data Expected: " <<  this->timestamp->back() + info.intervalInSeconds() << " Got: " << timestamp << std::endl;
                double lastClose = this->close->back();
                this->timestamp->push_back(this->timestamp->back() + info.intervalInSeconds());
                this->open->push_back(lastClose);
                this->high->push_back(lastClose);
                this->low->push_back(lastClose);
                this->close->push_back(lastClose);
                this->volume->push_back(0.0);
                this->median->push_back((lastClose + lastClose)/2.0);
                this->typical->push_back((lastClose + lastClose + lastClose)/3.0);
                this->weighted->push_back((lastClose + lastClose + 2 * lastClose)/4.0);
                this->zero->push_back(0.0);
            }
        }

        this->timestamp->push_back(timestamp);
        this->open->push_back(open);
        this->high->push_back(high);
        this->low->push_back(low);
        this->close->push_back(close);
        this->volume->push_back(volume);
        this->median->push_back((high + low)/2.0);
        this->typical->push_back((high + low + close)/3.0);
        this->weighted->push_back((high + low + 2 * close)/4.0);
        this->zero->push_back(0.0);
    }

};

class DataStore {
    fs::path storePath{};
    json dataJson;
    DataSetV2 dataset;

    void load(){
        auto api =  BinanceAPI{};
        auto filePath = path();
        if(fs::exists(filePath)){
            dataJson = JsonFileHandler::read(filePath.string());
        } else {
            dataJson = api.getOHLCVdata(dataset.info.symbol(), dataset.info.interval, -1);
            std::cout << std::setw(2) << dataJson << dataJson.size() << std::endl;
            JsonFileHandler::write(filePath.string(), dataJson);
        }
        update();
    }

    void updateDataJson(){
        auto api =  BinanceAPI{};
        int numUpdates = 0;
        while (isStale()){
            long fromTimeStamp = lastTimestamp() - dataset.info.intervalInSeconds();
            json newData = api.getOHLCVdata(dataset.info.symbol(), dataset.info.interval, fromTimeStamp);
            for(auto& row: newData) {
                int timestamp = row[0].get<long>()/1000;
                if(timestamp > lastTimestamp()){
                    dataJson.push_back(row);
                    numUpdates += 1;
                }
            }

            if(numUpdates >= 10000){
                JsonFileHandler::write(path().string(), dataJson);
                numUpdates = 0;
            }
        }
        if(numUpdates > 1000){
            JsonFileHandler::write(path().string(), dataJson);
        }
    }

    void updateDataset(){
        double open, high, low, close, volume;
        long timestamp;
        for(auto& row: dataJson){
            timestamp = row[0].get<long>()/1000;
            if(dataset.timestamp->size() > 0){
                long lastTimeStep = dataset.timestamp->back();
                if(timestamp < lastTimeStep){
                    continue;
                }
            }
            open = atof(row[1].get<std::string>().c_str());
            high = atof(row[2].get<std::string>().c_str());
            low = atof(row[3].get<std::string>().c_str());
            close = atof(row[4].get<std::string>().c_str());
            volume = atof(row[5].get<std::string>().c_str());
            dataset.add(timestamp, open, high, low, close, volume);
//            std::cout << std::setw(2) << row << std::endl;
        }
    }

    void update(){
        updateDataJson();
        updateDataset();
    }

    fs::path path(){
        auto fileName = storePath / (dataset.info.symbol() + "_" + dataset.info.intervalInString() + ".json");
        return fileName;
    }

    bool isStale(){
        const auto p1 = std::chrono::system_clock::now();
        long timeStampNow = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
        int intervalSeconds = dataset.info.intervalInSeconds();
        return lastTimestamp() < timeStampNow - intervalSeconds;
    }

    long lastTimestamp(){
        return dataJson.back()[0].get<long>()/1000;
    }

    int nextUpdateAt(){
        return lastTimestamp() + dataset.info.intervalInSeconds();
    }

    void getDataset(){

    }

public:
    DataStore(DataSetInfo info, std::string storePath): dataset(info){
        this->storePath = fs::path(storePath);
        load();
    }


};


#endif //CRYPTONITE_DATASTORE_H
