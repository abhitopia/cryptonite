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


#define NUM_BITS 16

unsigned int getNumberOfBitsNeeded(unsigned int range);


class Gene {
private:
    double _rangeMin{0.0};
    double _rangeMax{0.0};
    double _value{0.0};
    double _delta{1.0};
    double _numBits{0};
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

};


class DNA {
protected:
    unsigned int _numBits{0};
    map<string, Gene> _genes{};
    double _fitness{-std::numeric_limits<double>::infinity()};
    string getBitString() const;
    void setBitString(string bitString);

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

    void copyGenes(const DNA& copyFrom);;
    void recombineGenes(std::shared_ptr<DNA> parent1, std::shared_ptr<DNA> parent2);
    void mutateGenes(double probability=-1.0);;
    virtual void calcFitness() = 0;
};


#endif //CRYPTONITE_DNA_H