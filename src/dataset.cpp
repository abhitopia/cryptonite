//
// Created by Abhishek Aggarwal on 25/10/2021.
//
#include "dataset.h"


Dataset::Dataset(const DataSetContainer& container) {
    dataSetConfig = container.info;
    this->numBars = container.numBars();
    this->timestamp.reset(new long[this->numBars]);
    this->open.reset(new double[this->numBars]);
    this->high.reset(new double[this->numBars]);
    this->low.reset(new double[this->numBars]);
    this->close.reset(new double[this->numBars]);
    this->volume.reset(new double[this->numBars]);
    std::copy(container.timestamp->begin(), container.timestamp->end(), this->timestamp.get());
    std::copy(container.open->begin(), container.open->end(), this->open.get());
    std::copy(container.high->begin(), container.high->end(), this->high.get());
    std::copy(container.low->begin(), container.low->end(), this->low.get());
    std::copy(container.close->begin(), container.close->end(), this->close.get());
    std::copy(container.volume->begin(), container.volume->end(), this->volume.get());
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

void DataSetContainer::add(long timestamp, double open, double high, double low, double close, double volume) {
    if (this->timestamp->size() > 0) {
        while (this->timestamp->back() + info.intervalInSeconds() < timestamp) {
            double lastClose = this->close->back();
            this->timestamp->emplace_back(this->timestamp->back() + info.intervalInSeconds());
            this->open->emplace_back(lastClose);
            this->high->emplace_back(lastClose);
            this->low->emplace_back(lastClose);
            this->close->emplace_back(lastClose);
            this->volume->emplace_back(0.0);
        }

//            if(this->timestamp->back() + info.intervalInSeconds() != timestamp) {
//                std::cout << "Weird Encounter, should be " <<  this->timestamp->back() + info.intervalInSeconds() << " but got " <<  timestamp;
//            }
    }

    this->timestamp->emplace_back(timestamp);
    this->open->emplace_back(open);
    this->high->emplace_back(high);
    this->low->emplace_back(low);
    this->close->emplace_back(close);
    this->volume->emplace_back(volume);
}

int DataSetContainer::numBars() const {
    return this->timestamp->size();
}

void DataSetContainer::reserve(int n) {
    this->timestamp->reserve(n);
    this->open->reserve(n);
    this->high->reserve(n);
    this->low->reserve(n);
    this->close->reserve(n);
    this->volume->reserve(n);
}

Dataset DataSetContainer::dataset() {
    return Dataset(*this);
}
