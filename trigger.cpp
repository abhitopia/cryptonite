//
// Created by Abhishek Aggarwal on 26/10/2021.
//

#include "trigger.h"
using namespace std;

series_b Rises::compute(const int &len, series_d &comparand) {
    series_b result{new bool[len]};
    result[0] = false;
    for(int i=1; i < len; i++){
        result[i] = comparand[i] > comparand[i - 1];
    }
    return result;
}

shared_ptr<Trigger> Rises::get_contra() {
    return shared_ptr<Trigger>{new Falls(_comparand)};
}

series_b Falls::compute(const int &len, series_d &comparand) {
    series_b  result{new bool[len]};
    result[0] = false;

    for(int i=1; i < len; i++){
        result[i] = comparand[i] < comparand[i - 1];
    }
    return result;
}

shared_ptr<Trigger> Falls::get_contra() {
    return shared_ptr<Trigger>{new Rises(_comparand)};
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

shared_ptr<Trigger> ChangesDirectionUpward::get_contra() {
    return shared_ptr<Trigger>{new ChangesDirectionDownward(_comparand)};
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

shared_ptr<Trigger> ChangesDirectionDownward::get_contra() {
    return shared_ptr<Trigger>{new ChangesDirectionUpward(_comparand)};
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

shared_ptr<Trigger> HigherThan::get_contra() {
    return shared_ptr<Trigger>{new LowerThan(_comparand, _comparator)};
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

shared_ptr<Trigger> LowerThan::get_contra() {
    return shared_ptr<Trigger>{new HigherThan(_comparand, _comparator)};
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

shared_ptr<Trigger> CrossesUpward::get_contra() {
    return shared_ptr<Trigger>{new CrossesDownward(_comparand, _comparator)};
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

shared_ptr<Trigger> CrossesDownward::get_contra() {
    return shared_ptr<Trigger>{new CrossesUpward(_comparand, _comparator)};
}


vector<Trigger> higherLowerThanTriggers(string comparand, string comparator) {
    HigherThan trigger(comparand, comparator);
    LowerThan ctrigger(comparand, comparator);
    vector<Trigger> result{trigger, ctrigger};
    return result;
}

vector<Trigger> riseFallTriggers(string comparand) {
    Rises trigger(comparand);
    Falls ctrigger(comparand);
    vector<Trigger> result{trigger, ctrigger};
    return result;
}

vector<Trigger> directionChangeTriggers(string comparand) {
    ChangesDirectionUpward trigger(comparand);
    ChangesDirectionDownward ctrigger(comparand);
    vector<Trigger> result{trigger, ctrigger};
    return result;
}

std::vector<Trigger> operator+(const vector<Trigger> &v1, const vector<Trigger> &v2) {
    std::vector<Trigger> vr(std::begin(v1), std::end(v1));
    vr.insert(std::end(vr), std::begin(v2), std::end(v2));
    return vr;
}

vector<Trigger> crossingTriggers(string comparand, string comparator) {
    CrossesUpward trigger(comparand, comparator);
    CrossesDownward ctrigger(comparand, comparator);
    vector<Trigger> result{trigger, ctrigger};
    return result;
}

bool Trigger::has_level() {
    return _comparator == "level";
}

string Trigger::getComparand() {
    return _comparand;
}

string Trigger::getComparator() {
    return _comparator;
}
