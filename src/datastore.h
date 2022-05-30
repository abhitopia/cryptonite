//
// Created by Abhishek Aggarwal on 26/12/2021.
//

#ifndef CRYPTONITE_DATASTORE_H
#define CRYPTONITE_DATASTORE_H

#include <string>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include "binance.h"
#include "json_file_handler.h"
#include "../include/progress.hpp"
#include "config.h"
#include "dataset.h"


namespace fs = std::filesystem;

class DataSetManager {
    fs::path storePath{};
    DataSetContainer dataSetContainer{};
    json dataJson;

    void load(){
        auto api =  BinanceAPI{};
        auto filePath = path();
        if(fs::exists(filePath)){
            dataJson = JsonFileHandler::read(filePath.string());
        } else {
            dataJson = api.getOHLCVdata(dataSetContainer.info.symbol(), dataSetContainer.info.interval, -1);
            JsonFileHandler::write(filePath.string(), dataJson);
        }
        update();
    }

    void updateDataJson(){
        auto api =  BinanceAPI{};
        int numUpdates = 0;

        long timeStampNow = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        int numBars = (timeStampNow - lastTimestamp()) / dataSetContainer.info.intervalInSeconds();

        using namespace indicators;
        show_console_cursor(false);
        show_console_cursor(false);
        indicators::ProgressBar bar{
                option::BarWidth{100},
                option::Start{"\rDownloading data ["},
                option::Fill{"█"},
                option::Lead{"█"},
                option::Remainder{"-"},
                option::End{"]"},
                option::MaxProgress {numBars},
                option::ForegroundColor{Color::white},
                option::FontStyles{std::vector<FontStyle>{FontStyle::bold}},
        };
        bool showBar = numBars > 2000;
        while (isStale()){
            long fromTimeStamp = lastTimestamp() - dataSetContainer.info.intervalInSeconds();
            json newData = api.getOHLCVdata(dataSetContainer.info.symbol(), dataSetContainer.info.interval, fromTimeStamp);
            for(auto& row: newData) {
                int timestamp = row[0].get<long>()/1000;
                if(timestamp > lastTimestamp()){
                    if(showBar){
                        bar.tick();
                    }

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
        // Show cursor
        indicators::show_console_cursor(true);
        std::cout << std::endl;
    }

    void updateDataset(){
        dataSetContainer.resize(dataJson.size());

#pragma omp parallel for default(none) if(MULTITHREADED)
        for(int i=0; i<dataJson.size(); i++){
            auto row = dataJson[i];
            long timestamp = row[0].get<long>()/1000;
            double open = atof(row[1].get<std::string>().c_str());
            double high = atof(row[2].get<std::string>().c_str());
            double low = atof(row[3].get<std::string>().c_str());
            double close = atof(row[4].get<std::string>().c_str());
            double volume = atof(row[5].get<std::string>().c_str());
            dataSetContainer.set(i, timestamp, open, high, low, close, volume);
        }
    }

    fs::path path(){
        auto fileName = storePath / (dataSetContainer.info.symbol() + "_" + dataSetContainer.info.intervalInString() + ".json");
        return fileName;
    }

    bool isStale(){
        const auto p1 = std::chrono::system_clock::now();
        long timeStampNow = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
        int intervalSeconds = dataSetContainer.info.intervalInSeconds();
        return lastTimestamp() < timeStampNow - intervalSeconds;
    }

    long lastTimestamp(){
        return dataJson.back()[0].get<long>()/1000;
    }

    int nextUpdateAt(){
        return lastTimestamp() + dataSetContainer.info.intervalInSeconds();
    }
public:

    DataSetManager(){};
    DataSetManager(DataSetConfig info, std::string storePath): dataSetContainer(info){
            this->storePath = fs::path(storePath);
            load();
    }

    Dataset getDataset(){
        return dataSetContainer.dataset();
    }

    void update(){
        updateDataJson();
        updateDataset();
    }

};



class DataStore {
    fs::path storePath{};
    std::unordered_map<std::string, DataSetManager> datasetManagerMap{};

    std::string datasetConfigToKey(const DataSetConfig& datasetConfig){
        auto j = datasetConfig.toJson();
        return j["quoteAsset"].get<std::string>() + j["baseAsset"].get<std::string>() + j["interval"].get<std::string>();
    }

public:
    DataStore(){};
    DataStore(std::string storePath){
        this->storePath = fs::path(storePath);
    }

    std::string addDataset(const DataSetConfig& datasetConfig){
        auto key = datasetConfigToKey(datasetConfig);
        if(datasetManagerMap.count(key) == 0){
            datasetManagerMap[key] = DataSetManager(datasetConfig, this->storePath.string());
        }
        return key;
    }

    void updateDataset(const DataSetConfig& dataSetConfig){
        auto key = addDataset(dataSetConfig);
        datasetManagerMap[key].update();
    }

    Dataset getDataset(const DataSetConfig& datasetConfig){
        auto key = addDataset(datasetConfig);
        return datasetManagerMap[key].getDataset();
    }
};


#endif //CRYPTONITE_DATASTORE_H
