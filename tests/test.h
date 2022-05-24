//
// Created by Abhishek Aggarwal on 23/05/2022.
//

#ifndef CRYPTONITE_TEST_H
#define CRYPTONITE_TEST_H

#include "../src/random.h"
#include "../src/dna.h"
#include "../src/genetic_algorithm.hpp"

class TestDNA : public DNA {

public:
    double target{cryptonite::randint(0, 80)};
    TestDNA(map<std::string, Gene>& genes): DNA(genes){};
    TestDNA(): DNA(){};

    void calcFitness(){
        _fitness = 0;
        for(auto& it: _genes){
            _fitness -= std::pow(target - it.second.getValue(), 2);
        }
    }

    static TestDNA getTestDNA(int numVars=10){
        map<string, Gene> genes{};
        for(int i=0; i < numVars; i++){
            genes["var" + std::to_string(i)] = Gene{-40, -80.0, 80 };
        }
        return TestDNA(genes);
    }
};

void test_genetic_algorithm();




#endif //CRYPTONITE_TEST_H
