//
// Created by Abhishek Aggarwal on 25/10/2021.
//

#ifndef CRYPTONITE_DATASET_H
#define CRYPTONITE_DATASET_H

#include <string>
#include <memory>

using namespace std;

class Dataset {
public:
    int num_bars{};
    unique_ptr<int[]> timestamp{nullptr};
    unique_ptr<double[]> open{nullptr};
    unique_ptr<double[]> high{nullptr};
    unique_ptr<double[]> low{nullptr};
    unique_ptr<double[]> close{nullptr};
    unique_ptr<double[]> volume{nullptr};

    Dataset() = delete;
    Dataset(int num_bars, int *timestamp, double *open, double *high, double *low, double *close, double *volume);
    static Dataset from_csv(std::string);
};

#endif //CRYPTONITE_DATASET_H
