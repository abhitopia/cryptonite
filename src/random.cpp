//
// Created by Abhishek Aggarwal on 28/10/2021.
//

#include "random.h"

namespace cryptonite{
    RandomGenerator::RandomGenerator() {
        max_threads = omp_get_max_threads();
        generators.reset(new mt19937[max_threads]);
    }

    shared_ptr<RandomGenerator> _Random{new RandomGenerator{}};


    void seed(const int &seed) {
        int i;
        unsigned int _seed;
#pragma omp parallel for private (i, _seed) default(none) firstprivate(seed) shared(cout, _Random)
        for(i=0; i < _Random->max_threads; i++){
            if(seed < 0){
                //create seed on thread using current time
                _seed = ((unsigned) time(nullptr) + i);  // & (0xFFFFFFF0 | i);
            } else{
                _seed = seed + i;
            }
            _Random->generators[i].seed(_seed);
        }
    }

    double randn() {
        int gen_id = omp_get_thread_num() % _Random->max_threads;
        return _Random->normal_distribution(_Random->generators[gen_id]);
    }

    double randn(const double &mean, const double &sigma) {
        return mean + sigma * randn();
    }

    double rand(const double &min, const double &max) {
        int gen_id = omp_get_thread_num() % _Random->max_threads;
        return min + _Random->u_real_distribution(_Random->generators[gen_id]) * max;
    }

    double randint(const int &min, const int &max) {
        int gen_id = omp_get_thread_num() % _Random->max_threads;
        return min + _Random->u_int_distribution(_Random->generators[gen_id]) % (max - min);
    }

    double randint(const int &max) {
        return randint(0, max);
    }
}




