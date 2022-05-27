//
// Created by Abhishek Aggarwal on 16/05/2022.
//

#ifndef CRYPTONITE_DNA_H
#define CRYPTONITE_DNA_H

#include <cassert>

#include <string>
using std::string;

#include <utility>
#include <vector>
using std::vector;

#include <map>
using std::map;

#include <bitset>
#include <iostream>


#include <memory>
using std::unique_ptr;

#include "random.h"
#include "strategy.h"
#include "backtest.h"


#define NUM_BITS 16

unsigned int getNumberOfBitsNeeded(unsigned int range);


class Gene {
private:
    double _rangeMin{0.0};
    double _rangeMax{0.0};
    double _value{0.0};
    double _delta{1.0};
    unsigned int _steps{0};
    double _numBits{0};
    string _maxBits{""};
    string _bits{""};

public:
    Gene(double initValue, double rangeMin, double rangeMax, double delta=1.0);
    Gene(){}; // this is needed to be able to use in std::map
    double getValue() const;
    string getBitString() const;
    int getNumBits() const;
    void setBitString(string bits);
    void initRandomValue();
    void mutate(double probability = -1.0);
    void recombine(const Gene& gene1, const Gene& gene2);
    bool isValidBitString(string bitString);
};


class DNA {
protected:
    unsigned int _numBits{0};
    map<string, Gene> _genes{};
    double _fitness{-std::numeric_limits<double>::max()}; // important to not use -ve ::inf() as release precision will cause problems

//    string getBitString() const;
//    void setBitString(string bitString);

public:
    DNA(map<string, Gene> genes = {});
    DNA(const DNA& copyFrom);
    double getFitness() const;
    unsigned int getNumBits() const;
    DNA& operator = (const DNA& copyFrom);
    void initGenesWithRandomValues();

//    void showValues(){
//        for (auto& it: _genes){
//            std::cout << it.first << " " << it.second.getValue() << std::endl;
//        }
//    }

    void copyGenes(const DNA& copyFrom);
    void recombineGenes(std::shared_ptr<DNA> parent1, std::shared_ptr<DNA> parent2);
    void mutateGenes(double probability=-1.0);
    virtual void calcFitness() = 0;
};


class StrategyDNA: public DNA {
    std::shared_ptr<Backtester> _backtester{nullptr};
    std::shared_ptr<Strategy> _strategy{nullptr};

    void initGenes(){
        json j = _strategy->toJson().flatten();
        std::cout << j.dump(4) << std::endl;

        for(auto& [path, value]: j.items()) {
            if (path.find("/params/") != std::string::npos){
                if(path.find("/apply_to") != std::string::npos || path.find("/ma_method") != std::string::npos){
                    continue;
                }
                std::cout << path << " " << value << std::endl;
            } else if(path == "/positionCloseConfig/stopLoss" && value.get<double>() > 0){
                    std::cout << path << " " << value << std::endl;
            } else if(path == "/positionCloseConfig/takeProfit" && value.get<double>() > 0){
                    std::cout << path << " " << value << std::endl;
            }

        }
        int doSOmething = 1;
    }

public:
    StrategyDNA(map<string, Gene> genes = {}): DNA(genes){};
    StrategyDNA(std::shared_ptr<Backtester> backtester, std::shared_ptr<Strategy> strategy){
        _backtester = backtester;
        _strategy = strategy;
        initGenes();
    }

    void calcFitness() override {

    }
};

#endif //CRYPTONITE_DNA_H
