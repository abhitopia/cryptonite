//
// Created by Abhishek Aggarwal on 25/10/2021.
//

#ifndef CRYPTONITE_DATASET_H
#define CRYPTONITE_DATASET_H

#include "function.h"
#include "config.h"


struct DataSetContainer;

class Dataset {
public:
    int numBars{};
    std::shared_ptr<long[]> timestamp{nullptr};
    std::shared_ptr<double[]> open{nullptr};
    std::shared_ptr<double[]> high{nullptr};
    std::shared_ptr<double[]> low{nullptr};
    std::shared_ptr<double[]> close{nullptr};
    std::shared_ptr<double[]> volume{nullptr};
    std::shared_ptr<double[]> median{nullptr};
    std::shared_ptr<double[]> typical{nullptr};
    std::shared_ptr<double[]> weighted{nullptr};
    std::shared_ptr<double[]> zero{nullptr};

    Dataset() = delete;
    Dataset(const DataSetContainer& container);
    int durationSeconds() const{
        return timestamp[numBars - 1] - timestamp[0];
    }

    double durationDays() const {
        return durationSeconds() / (24.0 * 60 * 60);
    }

    int intervalSeconds() const{
        return timestamp[1] - timestamp[0];
    }

    int barsWithVolume() {
        int total = 0.0;
        for(int i=0; i<numBars; i++){
            if(volume[i] > 0){
                total += 1;
            }
        }
        return total;
    }
};

struct DataSetContainer {
    DataSetConfig info;
    std::shared_ptr<std::vector<long>> timestamp = std::make_shared<std::vector<long>>();
    std::shared_ptr<std::vector<double>> open = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> high = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> low = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> close = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> volume = std::make_shared<std::vector<double>>();

    DataSetContainer(DataSetConfig info) : info{info} {};

    void add(long timestamp, double open, double high, double low, double close, double volume) {
        if (this->timestamp->size() > 0) {
            while (this->timestamp->back() + info.intervalInSeconds() < timestamp) {
                double lastClose = this->close->back();
                this->timestamp->emplace_back(this->timestamp->back() + info.intervalInSeconds());
                this->open->emplace_back(lastClose);
                this->high->emplace_back(lastClose);
                this->low->emplace_back(lastClose);
                this->close->emplace_back(lastClose);
                this->volume->emplace_back(0.0);
            }

//            if(this->timestamp->back() + info.intervalInSeconds() != timestamp) {
//                std::cout << "Weird Encounter, should be " <<  this->timestamp->back() + info.intervalInSeconds() << " but got " <<  timestamp;
//            }
        }

        this->timestamp->emplace_back(timestamp);
        this->open->emplace_back(open);
        this->high->emplace_back(high);
        this->low->emplace_back(low);
        this->close->emplace_back(close);
        this->volume->emplace_back(volume);
    }

    int numBars() const {
        return this->timestamp->size();
    }

    void reserve(int n) {
        this->timestamp->reserve(n);
        this->open->reserve(n);
        this->high->reserve(n);
        this->low->reserve(n);
        this->close->reserve(n);
        this->volume->reserve(n);
    }

    Dataset dataset() {
        return Dataset(*this);
    }

};

#endif //CRYPTONITE_DATASET_H
