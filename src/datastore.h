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
#include "../include/progress.hpp"
#include "config.h"
#include "dataset.h"


namespace fs = std::filesystem;



class DataStore {
    fs::path storePath{};
    json dataJson;
    DataSetContainer dataSetContainer;

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
        double open, high, low, close, volume;
        long timestamp;

        if(dataJson.size() - dataSetContainer.numBars() > 50000){
            dataSetContainer.reserve(dataJson.size());
        }

        const int maxProgress = dataJson.size();
        using namespace indicators;
        show_console_cursor(false);
        indicators::ProgressBar bar{
                option::BarWidth{100},
                option::Start{"\rLoading data     ["},
                option::Fill{"█"},
                option::Lead{"█"},
                option::Remainder{"-"},
                option::End{"]"},
                option::MaxProgress {maxProgress},
                option::ForegroundColor{Color::white},
                option::FontStyles{std::vector<FontStyle>{FontStyle::bold}},
        };
        for(auto& row: dataJson){
                bar.tick();
            timestamp = row[0].get<long>()/1000;
            if(dataSetContainer.timestamp->size() > 0){
                long lastTimeStep = dataSetContainer.timestamp->back();
                if(timestamp < lastTimeStep){
                    continue;
                }
            }
            open = atof(row[1].get<std::string>().c_str());
            high = atof(row[2].get<std::string>().c_str());
            low = atof(row[3].get<std::string>().c_str());
            close = atof(row[4].get<std::string>().c_str());
            volume = atof(row[5].get<std::string>().c_str());
            dataSetContainer.add(timestamp, open, high, low, close, volume);
        }

        // Show cursor
        indicators::show_console_cursor(true);
        std::cout << std::endl;
    }

    void update(){
        updateDataJson();
        updateDataset();
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
    DataStore(DataSetConfig info, std::string storePath): dataSetContainer(info){
        this->storePath = fs::path(storePath);
        load();
    }

    Dataset getDataset(){
        return dataSetContainer.dataset();
    }

};


#endif //CRYPTONITE_DATASTORE_H
