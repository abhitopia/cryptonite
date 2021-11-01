//
// Created by Abhishek Aggarwal on 26/10/2021.
//

#include "trigger.h"
using namespace std;

series_b Rises::operator()(const int &len, series_d &comparand) {
    series_b result{new bool[len]};
    result[0] = false;
    for(int i=1; i < len; i++){
        result[i] = comparand[i] > comparand[i - 1];
    }
    return result;
}

Falls Rises::get_contra() {
    return Falls(_comparand);
}

series_b Falls::operator()(const int &len, series_d &comparand) {
    series_b  result{new bool[len]};
    result[0] = false;

    for(int i=1; i < len; i++){
        result[i] = comparand[i] < comparand[i - 1];
    }
    return result;
}

Rises Falls::get_contra() {
    return Rises(_comparand);
}


series_b ChangesDirectionUpward::operator()(const int &len, series_d &comparand) {
    series_b  result{new bool[len]};
    result[0] = false;
    result[1] = false;

    for(int i=2; i < len; i++){
        result[i] = comparand[i] > comparand[i - 1] && comparand[i - 1] < comparand[i - 2];
    }
    return result;
}

ChangesDirectionDownward ChangesDirectionUpward::get_contra() {
    return ChangesDirectionDownward(_comparand);
}

series_b ChangesDirectionDownward::operator()(const int &len, series_d &comparand) {
    series_b  result{new bool[len]};
    result[0] = false;
    result[1] = false;

    for(int i=2; i < len; i++){
        result[i] = comparand[i] < comparand[i - 1] && comparand[i - 1] > comparand[i - 2];
    }
    return result;
}

ChangesDirectionUpward ChangesDirectionDownward::get_contra() {
    return ChangesDirectionUpward(_comparand);
}

series_b HigherThan::operator()(const int &len, series_d &comparand, series_d &comparator) {
    series_b  result{new bool[len]};

    for(int i=0; i < len; i++){
        result[i] = comparand[i] > comparator[i];
    }
    return result;
}

series_b HigherThan::operator()(const int &len, series_d &comparand, double &comparator) {
    series_b  result{new bool[len]};

    for(int i=0; i < len; i++){
        result[i] = comparand[i] > comparator;
    }
    return result;
}

LowerThan HigherThan::get_contra() {
    return LowerThan(_comparand, _comparator);
}

series_b LowerThan::operator()(const int &len, series_d &comparand, double &comparator) {
    series_b  result{new bool[len]};

    for(int i=0; i < len; i++){
        result[i] = comparand[i] < comparator;
    }
    return result;
}

series_b LowerThan::operator()(const int &len, series_d &comparand, series_d &comparator) {
    series_b  result{new bool[len]};

    for(int i=0; i < len; i++){
        result[i] = comparand[i] < comparator[i];
    }
    return result;
}

HigherThan LowerThan::get_contra() {
    return HigherThan(_comparand, _comparator);
}

series_b CrossesUpward::operator()(const int &len, series_d &comparand, series_d &comparator) {
    series_b  result{new bool[len]};
    result[0] = false;
    for(int i=1; i < len; i++){
        result[i] = comparand[i] > comparator[i] && comparand[i-1] < comparator[i-1];
    }
    return result;
}

series_b CrossesUpward::operator()(const int &len, series_d &comparand, double &comparator) {
    series_b  result{new bool[len]};
    result[0] = false;
    for(int i=1; i < len; i++){
        result[i] = comparand[i] > comparator && comparand[i-1] < comparator;
    }
    return result;
}

CrossesDownward CrossesUpward::get_contra() {
    return CrossesDownward(_comparand, _comparator);
}


series_b CrossesDownward::operator()(const int &len, series_d &comparand, series_d &comparator) {
    series_b  result{new bool[len]};
    result[0] = false;
    for(int i=1; i < len; i++){
        result[i] = comparand[i] < comparator[i] && comparand[i-1] > comparator[i-1];
    }
    return result;
}

series_b CrossesDownward::operator()(const int &len, series_d &comparand, double &comparator) {
    series_b  result{new bool[len]};
    result[0] = false;
    for(int i=1; i < len; i++){
        result[i] = comparand[i] < comparator && comparand[i-1] > comparator;
    }
    return result;
}

CrossesUpward CrossesDownward::get_contra() {
    return CrossesUpward(_comparand, _comparator);
}


vector<Trigger> higherLowerThanTriggers(string comparand, string comparator) {
    HigherThan trigger(comparand, comparator);
    vector<Trigger> result = {trigger, trigger.get_contra()};
    return result;
}

vector<Trigger> riseFallTriggers(string comparand) {
    Rises trigger(comparand);
    vector<Trigger> result = {trigger, trigger.get_contra()};
    return result;
}

vector<Trigger> directionChangeTriggers(string comparand) {
    ChangesDirectionUpward trigger(comparand);
    vector<Trigger> result = {trigger, trigger.get_contra()};
    return result;
}

std::vector<Trigger> operator+(const vector<Trigger> &v1, const vector<Trigger> &v2) {
    std::vector<Trigger> vr(std::begin(v1), std::end(v1));
    vr.insert(std::end(vr), std::begin(v2), std::end(v2));
    return vr;
}

vector<Trigger> crossingTriggers(string comparand, string comparator) {
    CrossesUpward trigger(comparand, comparator);
    vector<Trigger> result = {trigger, trigger.get_contra()};
    return result;
}

bool Trigger::has_level() {
    return _comparator == "level";
}

string Trigger::getComparand() {
    return _comparand;
}
