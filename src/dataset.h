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
    DataSetConfig dataSetConfig{};
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

    Dataset(const DataSetContainer& container);
    int durationSeconds() const;
    double durationDays() const;
    int intervalSeconds() const;
    int barsWithVolume();
};

struct DataSetContainer {
    DataSetConfig info{};
    std::shared_ptr<std::vector<long>> timestamp = std::make_shared<std::vector<long>>();
    std::shared_ptr<std::vector<double>> open = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> high = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> low = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> close = std::make_shared<std::vector<double>>();
    std::shared_ptr<std::vector<double>> volume = std::make_shared<std::vector<double>>();

    DataSetContainer(){};
    DataSetContainer(const DataSetConfig& info) : info{info} {};
    void set(size_t index, long timestamp, double open, double high, double low, double close, double volume);
    int numBars() const;
    void resize(int n);
    Dataset dataset();
};

#endif //CRYPTONITE_DATASET_H
