//
// Created by Abhishek Aggarwal on 26/10/2021.
//

#include "trigger.h"

series_b Rises::compute(const int &len, series_d &comparand) {
    series_b result{new bool[len]};
    result[0] = false;
    for(int i=1; i < len; i++){
        result[i] = comparand[i] > comparand[i - 1];
    }
    return result;
}

std::shared_ptr<Trigger> Rises::get_contra() {
    return std::shared_ptr<Trigger>{new Falls(_comparand)};
}

series_b Falls::compute(const int &len, series_d &comparand) {
    series_b  result{new bool[len]};
    result[0] = false;

    for(int i=1; i < len; i++){
        result[i] = comparand[i] < comparand[i - 1];
    }
    return result;
}

std::shared_ptr<Trigger> Falls::get_contra() {
    return std::shared_ptr<Trigger>{new Rises(_comparand)};
}


series_b ChangesDirectionUpward::compute(const int &len, series_d &comparand) {
    series_b  result{new bool[len]};
    result[0] = false;
    result[1] = false;

    for(int i=2; i < len; i++){
        result[i] = comparand[i] > comparand[i - 1] && comparand[i - 1] < comparand[i - 2];
    }
    return result;
}

std::shared_ptr<Trigger> ChangesDirectionUpward::get_contra() {
    return std::shared_ptr<Trigger>{new ChangesDirectionDownward(_comparand)};
}

series_b ChangesDirectionDownward::compute(const int &len, series_d &comparand) {
    series_b  result{new bool[len]};
    result[0] = false;
    result[1] = false;

    for(int i=2; i < len; i++){
        result[i] = comparand[i] < comparand[i - 1] && comparand[i - 1] > comparand[i - 2];
    }
    return result;
}

std::shared_ptr<Trigger> ChangesDirectionDownward::get_contra() {
    return std::shared_ptr<Trigger>{new ChangesDirectionUpward(_comparand)};
}

series_b HigherThan::compute(const int &len, series_d &comparand, series_d &comparator) {
    series_b  result{new bool[len]};

    for(int i=0; i < len; i++){
        result[i] = comparand[i] > comparator[i];
    }
    return result;
}

series_b HigherThan::compute(const int &len, series_d &comparand, double &comparator) {
    series_b  result{new bool[len]};

    for(int i=0; i < len; i++){
        result[i] = comparand[i] > comparator;
    }
    return result;
}

std::shared_ptr<Trigger> HigherThan::get_contra() {
    return std::shared_ptr<Trigger>{new LowerThan(_comparand, _comparator)};
}

series_b LowerThan::compute(const int &len, series_d &comparand, double &comparator) {
    series_b  result{new bool[len]};

    for(int i=0; i < len; i++){
        result[i] = comparand[i] < comparator;
    }
    return result;
}

series_b LowerThan::compute(const int &len, series_d &comparand, series_d &comparator) {
    series_b  result{new bool[len]};

    for(int i=0; i < len; i++){
        result[i] = comparand[i] < comparator[i];
    }
    return result;
}

std::shared_ptr<Trigger> LowerThan::get_contra() {
    return std::shared_ptr<Trigger>{new HigherThan(_comparand, _comparator)};
}

series_b CrossesUpward::compute(const int &len, series_d &comparand, series_d &comparator) {
    series_b  result{new bool[len]};
    result[0] = false;
    for(int i=1; i < len; i++){
        result[i] = comparand[i] > comparator[i] && comparand[i-1] < comparator[i-1];
    }
    return result;
}

series_b CrossesUpward::compute(const int &len, series_d &comparand, double &comparator) {
    series_b  result{new bool[len]};
    result[0] = false;
    for(int i=1; i < len; i++){
        result[i] = comparand[i] > comparator && comparand[i-1] < comparator;
    }
    return result;
}

std::shared_ptr<Trigger> CrossesUpward::get_contra() {
    return std::shared_ptr<Trigger>{new CrossesDownward(_comparand, _comparator)};
}


series_b CrossesDownward::compute(const int &len, series_d &comparand, series_d &comparator) {
    series_b  result{new bool[len]};
    result[0] = false;
    for(int i=1; i < len; i++){
        result[i] = comparand[i] < comparator[i] && comparand[i-1] > comparator[i-1];
    }
    return result;
}

series_b CrossesDownward::compute(const int &len, series_d &comparand, double &comparator) {
    series_b  result{new bool[len]};
    result[0] = false;
    for(int i=1; i < len; i++){
        result[i] = comparand[i] < comparator && comparand[i-1] > comparator;
    }
    return result;
}

std::shared_ptr<Trigger> CrossesDownward::get_contra() {
    return std::shared_ptr<Trigger>{new CrossesUpward(_comparand, _comparator)};
}


std::vector<std::shared_ptr<Trigger>> higherLowerThanTriggers(std::string comparand, std::string comparator) {
    std::vector<std::shared_ptr<Trigger>> result;
    result.emplace_back(new HigherThan(comparand, comparator));
    result.emplace_back(new LowerThan(comparand, comparator));
    return result;
}

std::vector<std::shared_ptr<Trigger>> riseFallTriggers(std::string comparand) {
    std::vector<std::shared_ptr<Trigger>> result;
    result.emplace_back(new Rises(comparand));
    result.emplace_back(new Falls(comparand));
    return result;
}

std::vector<std::shared_ptr<Trigger>> directionChangeTriggers(std::string comparand) {
    std::vector<std::shared_ptr<Trigger>> result;
    result.emplace_back(new ChangesDirectionUpward(comparand));
    result.emplace_back(new ChangesDirectionDownward(comparand));
    return result;
}

std::vector<std::shared_ptr<Trigger>> crossingTriggers(std::string comparand, std::string comparator) {
    std::vector<std::shared_ptr<Trigger>> result;
    result.emplace_back(new CrossesUpward(comparand, comparator));
    result.emplace_back(new CrossesDownward(comparand, comparator));
    return result;
}

std::vector<std::shared_ptr<Trigger>> operator+(const std::vector<std::shared_ptr<Trigger>> &v1, const std::vector<std::shared_ptr<Trigger>> &v2) {
    std::vector<std::shared_ptr<Trigger>> vr(std::begin(v1), std::end(v1));
    vr.insert(std::end(vr), std::begin(v2), std::end(v2));
    return vr;
}

std::vector<std::shared_ptr<Trigger>> operator+(const std::vector<std::shared_ptr<Trigger>> &v1, const std::shared_ptr<Trigger> &v2) {
    std::vector<std::shared_ptr<Trigger>> vr(std::begin(v1), std::end(v1));
    vr.emplace_back(v2);
    return v1;
}

series_b UnimplementedError() {
    std::runtime_error("This function should never have been called!");
    return series_b{};
}


bool Trigger::has_level() {
    return _comparator == "level";
}

std::string Trigger::getComparand() {
    return _comparand;
}

std::string Trigger::getComparator() {
    return _comparator;
}

json Trigger::toJson() {
    json j;
    j["name"] = name;
    j["comparand"] = _comparand;
    j["comparator"] = _comparator;
    return j;
}
