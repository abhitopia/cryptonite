//
// Created by Abhishek Aggarwal on 28/10/2021.
//

#ifndef CRYPTONITE_RANDOM_H
#define CRYPTONITE_RANDOM_H
#include <iostream>
#include "omp.h"
#include <random>
#include <memory>
#include <limits>       // std::numeric_limits



namespace cryptonite {
    struct RandomGenerator {
        std::shared_ptr<std::mt19937[]> generators{nullptr};
        std::uniform_real_distribution<double> u_real_distribution{0.0, 1.0};
        std::uniform_int_distribution<int> u_int_distribution{0, std::numeric_limits<int>::max()};
        std::normal_distribution<double> normal_distribution{0.0, 1.0};
        int max_threads{};
        RandomGenerator();
    };

    extern std::shared_ptr<RandomGenerator> _Random;
    void seed(const int &seed=-1);
    double rand(const double &min=0.0, const double &max=1.0);
    double randint(const int &min, const int& max);
    double randint(const int& max);
    double randn();
    double randn(const double &mean, const double &sigma);
}

#endif //CRYPTONITE_RANDOM_H
