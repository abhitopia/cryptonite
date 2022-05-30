//
// Created by Abhishek Aggarwal on 16/05/2022.
//

#include "dna.h"

#include <iomanip>
using std::left;
using std::setw;
using std::fixed;
using std::setprecision;

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
    _steps = (int)std::round((_rangeMax - _rangeMin)/_delta);
    _numBits = getNumberOfBitsNeeded(_steps);
    std::bitset<NUM_BITS> maxBits(_steps);
    _maxBits = maxBits.to_string().substr(NUM_BITS -_numBits, _numBits);

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

    probability = probability < 0 || probability >= 1.0 ? 1.0/_numBits : probability;
    std::bitset<NUM_BITS> bits(_bits);
    for(int i=0; i < _numBits; i++){
        if(cryptonite::rand() < probability){
            bits[i].flip();
        }
    }
    string newBitString = bits.to_string().substr(NUM_BITS -_numBits, _numBits);
    if(!isValidBitString(newBitString)){
        return mutate(probability);
    }
    setBitString(newBitString);

}

bool Gene::isValidBitString(string bitString) {
    unsigned int maxBinNumber = std::stoi(_maxBits, 0, 2);
    return bitString.length() == _numBits && std::stoi(bitString, 0, 2) <= maxBinNumber;
}

void Gene::recombine(const Gene& gene1, const Gene& gene2) {
    size_t p = cryptonite::randint(0, _numBits);
    // 01010
    // p = 1 => dna1.substr(0, 2) + dna2.substr(2, 3)
    // p = 3 => dna1.substr(0, 4) + dna2.substr(4, 1)
    auto dna1 = gene1.getBitString();
    auto dna2 = gene2.getBitString();
    string newDna = dna1.substr(0, p+1) + dna2.substr(p+1, _numBits - p - 1);

    if(!isValidBitString(newDna)){
        return recombine(gene1, gene2);
    }
    setBitString(newDna);
}

//string DNA::getBitString() const {
//    string _bits = "";
//    for (auto& it: _genes){
//        _bits += it.second.getBitString();
//    }
//    return _bits;
//}
//
//void DNA::setBitString(string bitString) {
//    size_t p = 0;
//    for (auto& it: _genes){
//        bitString.substr(p, it.second.getNumBits());
//        it.second.setBitString(bitString.substr(0, it.second.getNumBits()));
//        p += it.second.getNumBits();
//    }
//}

DNA::DNA(map<string, Gene> genes) {
    _genes = genes;
    _numBits = 0;
    for (auto& it: genes){
        _numBits += it.second.getNumBits();
    }
}

DNA::DNA(const DNA &copyFrom) {
    this->copyGenes(copyFrom);
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
    // select crossover point uniformly distributed across the gene, independ of gene range (numBits)
    size_t g = cryptonite::randint(_genes.size());
    int gi = 0;
    for(auto it=_genes.begin(), pit1=parent1->_genes.begin(), pit2=parent2->_genes.begin(); it!=_genes.end();it++,pit1++, pit2++, gi++){
        if(gi < g) { // copy set parent 1 gene string
            it->second.setBitString(pit1->second.getBitString());
        } else if(gi > g) {
            it->second.setBitString(pit2->second.getBitString());
        }else if(gi == g){
            it->second.recombine(pit1->second, pit2->second);
        }
    }

}

void DNA::mutateGenes(double probability) {
    if(probability < 0.0 || probability >= 1.0){
        probability = 1.0/_numBits;
    }

    for (auto& it: _genes){
        it.second.mutate(probability);
    }
}

string DNA::getMetric() const {
    return std::to_string(_fitness);
}

void StrategyDNA::initGenesFromStrategyJson() {
    json j = _strategyJson.flatten();
//        std::cout << j.dump(4) << std::endl;
    for(auto& [path, value]: j.items()) {
        if (path.find("/params/") != std::string::npos){
            if(path.find("/apply_to") != std::string::npos || path.find("/ma_method") != std::string::npos){
                continue;
            }
            if(path.find("/level") != std::string::npos){
                double curValue = value.get<double>();
                double valMin = curValue - std::max(std::abs(curValue/5.0), 5.0);
                double valMax = curValue + std::max(std::abs(curValue/5.0), 5.0);
                double delta = (valMax - valMin)/100.0;
                _genes[path] = Gene(value.get<double>(), valMin, valMax, delta);
            } else if(path.find("pct") != std::string::npos){
                _genes[path] = Gene(value.get<double>(), std::max(1.0, value.get<double>()-10) , std::min(100.0, value.get<double>()+10));
            } else if(path.find("period") != std::string::npos){
                _genes[path] = Gene(value.get<double>(), std::max(2.0, value.get<double>()-10), value.get<double>()+10);
            } else if(path.find("shift") != std::string::npos){
                _genes[path] = Gene(value.get<double>(), std::max(0.0, value.get<double>()-5), value.get<double>()+5);
            }
//                std::cout << path << " " << value << std::endl;
        } else if(path == "/positionCloseConfig/stopLoss" && !value.is_null() && value.get<double>() > 0){
//                    std::cout << path << " " << value << std::endl;
            _genes[path] = Gene(value.get<double>(), .01, 0.3, 0.005);
        } else if(path == "/positionCloseConfig/takeProfit" && !value.is_null() && value.get<double>() > 0){
//                    std::cout << path << " " << value << std::endl;
            _genes[path] = Gene(value.get<double>(), .01, 0.3, 0.005);
        }
    }

    // exception
    std::string fast_period_str = "/fast_period";
    for(const auto& [key, gene]: _genes){
        if(key.find(fast_period_str) != std::string::npos){
            auto slow_period_key = key.substr(0, key.length()-fast_period_str.length()) + "/slow_period";
            double fast_val = j[key].get<double>();
            double slow_val = j[slow_period_key].get<double>();
            double mid = std::round((fast_val + slow_val)/2.0);
            _genes[key] = Gene(fast_val, std::max(2.0, fast_val-10), mid);
            _genes[slow_period_key] = Gene(slow_val, mid, slow_val + 10);

        }
    }
}

json StrategyDNA::getStrategyJsonFromGenes() const {
    json j = _strategyJson.flatten();
    for(const auto& [key, gene]: _genes){
        j[key] = gene.getValue();
//        std::cout << key << " " << gene.getValue() << std::endl;
    }
    return j.unflatten();
}

StrategyDNA::StrategyDNA(std::shared_ptr<Backtester> backtester, json strategy) {
    _backtester = backtester;
    _strategyJson = strategy;
    initGenesFromStrategyJson();
    _numBits = 0;
    for (auto& it: _genes){
        _numBits += it.second.getNumBits();
    }
}

void StrategyDNA::calcFitness() {
    auto strategy = Strategy::fromJson(getStrategyJsonFromGenes());
    _backtest = std::make_shared<Backtest>(_backtester->evaluate(strategy));
    _fitness = _backtest->metrics.metric();
}

StrategyDNA::StrategyDNA(const StrategyDNA &copyFrom) {
    copyGenes(copyFrom);
}

void StrategyDNA::copyGenes(const StrategyDNA &copyFrom) {
    DNA::copyGenes(copyFrom);
    _backtester = copyFrom._backtester;
    _strategyJson = copyFrom._strategyJson;
}

string StrategyDNA::getMetric() const {
    string result = std::to_string(_fitness) + " ";
    json metrics = _backtest->metrics.toJson();
    result += "\tNum Trades: " + std::to_string(metrics["numTrades"].get<int>());
    result += "\tCAGROverAvgDrawDown: " + std::to_string(metrics["CAGROverAvgDrawDown"].get<double>());
    return result;
}

Backtest StrategyDNA::getBacktest() const {
    auto strategy = Strategy::fromJson(getStrategyJsonFromGenes());
    return _backtester->evaluate(strategy);
}
