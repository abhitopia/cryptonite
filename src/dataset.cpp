//
// Created by Abhishek Aggarwal on 25/10/2021.
//
#include "dataset.h"


Dataset::Dataset(const DataSetContainer& container, double noise) {
    assert(noise >= 0.0 and noise <= 0.05 and "noise level must be in [0.0, 0.05]");
    this->noise = noise;
    dataSetConfig = container.info;
    this->numBars = std::min<unsigned int>(container.numBars(), dataSetConfig.numBars);
    int offset =  container.numBars() - this->numBars;
    this->timestamp.reset(new long[this->numBars]);
    this->open.reset(new double[this->numBars]);
    this->high.reset(new double[this->numBars]);
    this->low.reset(new double[this->numBars]);
    this->close.reset(new double[this->numBars]);
    this->volume.reset(new double[this->numBars]);
    std::copy(container.timestamp->begin() + offset, container.timestamp->end(), this->timestamp.get());
    std::copy(container.open->begin() + offset, container.open->end(), this->open.get());
    std::copy(container.high->begin() + offset, container.high->end(), this->high.get());
    std::copy(container.low->begin() + offset, container.low->end(), this->low.get());
    std::copy(container.close->begin() + offset, container.close->end(), this->close.get());
    std::copy(container.volume->begin() + offset, container.volume->end(), this->volume.get());
    applyNoise();  // First apply the noise!

    this->median = CIndicator::medprice(this->numBars, {this->high, this->low})[0];
    this->typical = CIndicator::typprice(this->numBars, {this->high, this->low, this->close})[0];
    this->weighted = CIndicator::wcprice(this->numBars, {this->high, this->low, this->close})[0];
    this->zero = CIndicator::sub(this->numBars, {this->high, this->high})[0];
}

int Dataset::barsWithVolume() {
    int total = 0.0;
    for(int i=0; i<numBars; i++){
        if(volume[i] > 0){
            total += 1;
        }
    }
    return total;
}

int Dataset::intervalSeconds() const {
    return timestamp[1] - timestamp[0];
}

double Dataset::durationDays() const {
    return durationSeconds() / (24.0 * 60 * 60);
}

int Dataset::durationSeconds() const {
    return timestamp[numBars - 1] - timestamp[0];
}

void Dataset::applyNoise() {
    if(noise == 0.0){
        return;
    }

#pragma omp parallel for default(none) if(MULTITHREADED)
    for(int i=0; i<numBars; i++){
        double noiseNow = cryptonite::rand(1.0 - noise, 1 + noise);
        open[i] *= noiseNow;
        close[i] *= noiseNow;
        high[i] *= noiseNow;
        low[i] *= noiseNow;
        volume[i] *= noiseNow;
    }
}

void DataSetContainer::set(size_t index, long timestamp, double open, double high, double low, double close, double volume) {
    this->timestamp->at(index) = timestamp;
    this->open->at(index) = open;
    this->high->at(index) = high;
    this->low->at(index) = low;
    this->close->at(index) = close;
    this->volume->at(index) = volume;
}

int DataSetContainer::numBars() const {
    return this->timestamp->size();
}

Dataset DataSetContainer::dataset(double noise) {
    return Dataset(*this, noise);
}

void DataSetContainer::resize(int n) {
    this->timestamp->resize(n);
    this->open->resize(n);
    this->high->resize(n);
    this->low->resize(n);
    this->close->resize(n);
    this->volume->resize(n);
}