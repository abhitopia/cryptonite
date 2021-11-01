//
// Created by Abhishek Aggarwal on 25/10/2021.
//

#ifndef CRYPTONITE_DATASET_H
#define CRYPTONITE_DATASET_H

#include <string>
#include <memory>
#include "function.h"

using namespace std;

class Dataset {
public:
    int num_bars{};
    shared_ptr<int[]> timestamp{nullptr};
    shared_ptr<double[]> open{nullptr};
    shared_ptr<double[]> high{nullptr};
    shared_ptr<double[]> low{nullptr};
    shared_ptr<double[]> close{nullptr};
    shared_ptr<double[]> volume{nullptr};
    shared_ptr<double[]> median{nullptr};
    shared_ptr<double[]> typical{nullptr};
    shared_ptr<double[]> weighted{nullptr};

    Dataset() = delete;
    Dataset(int num_bars, int *timestamp, double *open, double *high, double *low, double *close, double *volume);
    static Dataset from_csv(std::string);
};

#endif //CRYPTONITE_DATASET_H
