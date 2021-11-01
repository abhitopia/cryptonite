//
// Created by Abhishek Aggarwal on 26/10/2021.
//

#ifndef CRYPTONITE_TRIGGER_H
#define CRYPTONITE_TRIGGER_H

#include <memory>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

typedef const std::shared_ptr<double[]> series_d;
typedef std::shared_ptr<bool[]> series_b;


class Trigger {
protected:
    string name = "";
    string _comparand = "";
    string _comparator = "";

public:
    Trigger() = delete;
    Trigger(string comparand, string comparator): _comparand(comparand), _comparator(comparator){};
    virtual series_b operator()(const int &len, series_d &comparand){};
    virtual series_b operator()(const int &len, series_d &comparand, series_d &comparator){};
    virtual series_b operator()(const int &len, double &comparand){};
    bool has_level();
    string getComparand();
    void to_json(){
        std::cout << "Trigger: " << name << endl;
    }
};

class Falls;
class Rises;
class HigherThan;
class LowerThan;
class CrossesUpward;
class CrossesDownward;
class ChangesDirectionUpward;
class ChangesDirectionDownward;

class Rises: public Trigger {
    Rises();
public:
    Rises(string comparand): Trigger(comparand, "") {
        name = "rises";
    };
    series_b operator()(const int &len, series_d &comparand);
    Falls get_contra();
};

class Falls: public Trigger {
    Falls();
public:
    Falls(string comparand): Trigger(comparand, "") {
        name="falls";
    };
    series_b operator()(const int &len, series_d &comparand);
    Rises get_contra();
};

class ChangesDirectionUpward : public Trigger {
    ChangesDirectionUpward();
public:
    ChangesDirectionUpward(string comparand): Trigger(comparand, "") {
        name="changes_direction_upward";
    };
    series_b operator()(const int &len, series_d &comparand);
    ChangesDirectionDownward get_contra();

};

class ChangesDirectionDownward : public Trigger {
    ChangesDirectionDownward();
public:
    ChangesDirectionDownward(string comparand): Trigger(comparand, "") {
        name="changes_direction_downward";
    };
    series_b operator()(const int &len, series_d &comparand);
    ChangesDirectionUpward get_contra();

};

class HigherThan : public Trigger {
    HigherThan();
public:
    HigherThan(string comparand, string comparator): Trigger(comparand, comparator) {
        name="higher_than";
    };
    series_b operator()(const int &len, series_d &comparand, series_d &comparator);
    series_b operator()(const int &len, series_d &comparand, double &comparator);
    LowerThan get_contra();

};

class LowerThan : public Trigger {
    LowerThan();
public:
    LowerThan(string comparand, string comparator): Trigger(comparand, comparator) {
        name="lower_than";
    };
    series_b operator()(const int &len, series_d &comparand, series_d &comparator);
    series_b operator()(const int &len, series_d &comparand, double &comparator);
    HigherThan get_contra();
};

class CrossesUpward : public Trigger {
    CrossesUpward();
public:
    CrossesUpward(string comparand, string comparator): Trigger(comparand, comparator) {
        name="crosses_upward";
    };
    series_b operator()(const int &len, series_d &comparand, series_d &comparator);
    series_b operator()(const int &len, series_d &comparand, double &comparator);
    CrossesDownward get_contra();
};

class CrossesDownward : public Trigger {
    CrossesDownward();
public:
    CrossesDownward(string comparand, string comparator): Trigger(comparand, comparator) {
        name="crosses_upward";
    };
    series_b operator()(const int &len, series_d &comparand, series_d &comparator);
    series_b operator()(const int &len, series_d &comparand, double &comparator);
    CrossesUpward get_contra();
};

vector<Trigger> higherLowerThanTriggers(string comparand, string comparator);
vector<Trigger> crossingTriggers(string comparand, string comparator);
vector<Trigger> riseFallTriggers(string comparand);

vector<Trigger> directionChangeTriggers(string comparand);

std::vector<Trigger> operator+(const std::vector<Trigger>& v1, const std::vector<Trigger>& v2);

#endif //CRYPTONITE_TRIGGER_H
