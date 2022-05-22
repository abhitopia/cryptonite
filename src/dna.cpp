//
// Created by Abhishek Aggarwal on 16/05/2022.
//

#include "dna.h"

unsigned int getNumberOfBitsNeeded(unsigned int range) {
    int bits = 0;
    for (int bit_test = 16; bit_test > 0; bit_test >>= 1)
    {
        if (range >> bit_test != 0)
        {
            bits += bit_test;
            range >>= bit_test;
        }
    }
    return bits + range;
}

double Gene::getValue() const {
    return _value;
}

Gene::Gene(double initValue, double rangeMin, double rangeMax, double delta) {
    _rangeMin = rangeMin;
    _rangeMax = rangeMax;
    _value = initValue;
    _delta = delta;
    assert(_rangeMax > _rangeMin && "rangeMax must be greater than rangeMin");
    assert(_rangeMax >= initValue && initValue >= _rangeMin && "value must be in [rangeMin, rangeMax]");
    _numBits = getNumberOfBitsNeeded((int)(_rangeMax - _rangeMin)/delta);
    assert(_numBits <= NUM_BITS && "Number of bits required must be less than 16. Range too wide!");
    _bits = std::string (_numBits, '0');
}

string Gene::getBitString() const {
    return _bits;
}

void Gene::setBitString(string bits) {
    _bits = bits;
    _value = _rangeMin + std::stoi(_bits, 0, 2) * _delta;
    if(_value > _rangeMax){
        _value = _value;
    }

}

int Gene::getNumBits() const {
    return _numBits;
}

void Gene::initRandomValue() {
    double distToBound = std::max(_rangeMax - _value, _value - _rangeMin);
    double sigma = distToBound/3.0;  // 2 sigma includes 95 % samples;
    double value = cryptonite::randn(_value, sigma); // dynamic double to int
    _value = std::max(std::min(value, _rangeMax), _rangeMin);
    int steps = (int)std::round((_value - _rangeMin)/_delta);
    std::bitset<NUM_BITS> bits(steps);
    string bitString = bits.to_string().substr(NUM_BITS -_numBits, _numBits);
    setBitString(bitString);
}

void Gene::mutate(double probability) {

    probability = _numBits < 0 || _numBits >= 1.0 ? 1.0/_numBits : probability;
    int maxSteps = (int)std::round((_rangeMax - _rangeMin)/_delta);
    std::bitset<NUM_BITS> maxBits(maxSteps);
    unsigned int maxBinNumber = std::stoi(maxBits.to_string(), 0, 2);
    std::bitset<NUM_BITS> bits;
    do{
        bits = std::bitset<NUM_BITS>(_bits);
        for(int i=0; i < _numBits; i++){
            if(cryptonite::rand() < probability){
                bits[i].flip();
            }
        }

    }while(maxBinNumber < std::stoi(bits.to_string(), 0, 2));

    if(_value > _rangeMax){
        _value = _value;
    }
    setBitString(bits.to_string().substr(NUM_BITS -_numBits, _numBits));
    if(_value > _rangeMax){
        _value = _value;
    }
}

string DNA::getBitString() const {
    string _bits = "";
    for (auto& it: _genes){
        _bits += it.second.getBitString();
    }
    return _bits;
}

void DNA::setBitString(string bitString) {
    size_t p = 0;
    for (auto& it: _genes){
        bitString.substr(p, it.second.getNumBits());
        it.second.setBitString(bitString.substr(0, it.second.getNumBits()));
        p += it.second.getNumBits();
    }
}

DNA::DNA(map<string, Gene> genes) {
    _genes = std::move(genes);
    _numBits = 0;
    for (auto& it: _genes){
        _numBits += it.second.getNumBits();
    }
}

DNA::DNA(const DNA &copyFrom) {
    copyGenes(copyFrom);
}

double DNA::getFitness() const {
    return _fitness;
}

unsigned int DNA::getNumBits() const {
    return _numBits;
}

DNA &DNA::operator=(const DNA &copyFrom) {
    copyGenes(copyFrom);
    return *this;
}

void DNA::initGenesWithRandomValues() {
    for (auto& it: _genes){
        it.second.initRandomValue();
    }
}

void DNA::copyGenes(const DNA &copyFrom) {
    _genes.clear();
    _genes.insert(copyFrom._genes.begin(), copyFrom._genes.end());
    _fitness = copyFrom._fitness;
    _numBits = copyFrom._numBits;
}

void DNA::recombineGenes(std::shared_ptr<DNA> parent1, std::shared_ptr<DNA> parent2) {
    string dna1 = parent1->getBitString();
    string dna2 = parent2->getBitString();
    // select crossover point that is not on the end of the string
    size_t p = cryptonite::randint(1, _numBits-1); // excluding max

    // 01010
    // p = 1 => dna1.substr(0, 2) + dna2.substr(2, 3)
    string newDna = dna1.substr(0, p+1) + dna2.substr(p+1, _numBits - p - 1);
    setBitString(newDna);
}

void DNA::mutateGenes(double probability) {
    if(probability < 0.0 || probability >= 1.0){
        probability = 1.0/_numBits;
    }

    for (auto& it: _genes){
        it.second.mutate(probability);
    }
}
