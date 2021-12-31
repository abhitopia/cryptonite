//
// Created by Abhishek Aggarwal on 25/10/2021.
//
#include "dataset.h"


Dataset::Dataset(const DataSetContainer& container) {
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

