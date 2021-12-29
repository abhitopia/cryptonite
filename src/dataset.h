//
// Created by Abhishek Aggarwal on 25/10/2021.
//

#ifndef CRYPTONITE_DATASET_H
#define CRYPTONITE_DATASET_H

#include <string>
#include <memory>
#include "function.h"

class Dataset {
public:
    int num_bars{};
    std::shared_ptr<int[]> timestamp{nullptr};
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
    Dataset(int num_bars, int *timestamp, double *open, double *high, double *low, double *close, double *volume);
    int durationSeconds() const{
        return timestamp[num_bars-1] - timestamp[0];
    }

    double durationDays() const {
        return durationSeconds() / (24.0 * 60 * 60);
    }

    int intervalSeconds() const{
        return timestamp[1] - timestamp[0];
    }


    static Dataset from_csv(std::string);
};

#endif //CRYPTONITE_DATASET_H
