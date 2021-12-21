//
// Created by Abhishek Aggarwal on 26/10/2021.
//

#ifndef CRYPTONITE_TRIGGER_H
#define CRYPTONITE_TRIGGER_H

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include "../include/json.h"

using json = nlohmann::json;
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
    virtual series_b compute(const int &len, series_d &comparand) = 0;
    virtual series_b compute(const int &len, series_d &comparand, series_d &comparator) = 0;
    virtual series_b compute(const int &len, series_d &comparand, double &comparator) = 0;
    virtual shared_ptr<Trigger> get_contra(){return nullptr;};
    bool has_level();
    string getComparand();
    string getComparator();
    json toJson();
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
    series_b compute(const int &len, series_d &comparand) override;
    series_b compute(const int &len, series_d &comparand, series_d &comparator){};
    series_b compute(const int &len, series_d &comparand, double &comparator){};
    shared_ptr<Trigger> get_contra() override;
};

class Falls: public Trigger {
    Falls();
public:
    Falls(string comparand): Trigger(comparand, "") {
        name="falls";
    };
    series_b compute(const int &len, series_d &comparand);
    series_b compute(const int &len, series_d &comparand, series_d &comparator){};
    series_b compute(const int &len, series_d &comparand, double &comparator){};
    shared_ptr<Trigger> get_contra() override;
};

class ChangesDirectionUpward : public Trigger {
    ChangesDirectionUpward();
public:
    ChangesDirectionUpward(string comparand): Trigger(comparand, "") {
        name="changes_direction_upward";
    };
    series_b compute(const int &len, series_d &comparand);
    series_b compute(const int &len, series_d &comparand, series_d &comparator){};
    series_b compute(const int &len, series_d &comparand, double &comparator){};
    shared_ptr<Trigger> get_contra() override;
};

class ChangesDirectionDownward : public Trigger {
    ChangesDirectionDownward();
public:
    ChangesDirectionDownward(string comparand): Trigger(comparand, "") {
        name="changes_direction_downward";
    };
    series_b compute(const int &len, series_d &comparand);
    series_b compute(const int &len, series_d &comparand, series_d &comparator){};
    series_b compute(const int &len, series_d &comparand, double &comparator){};
    shared_ptr<Trigger> get_contra() override;
};

class HigherThan : public Trigger {
    HigherThan();
public:
    HigherThan(string comparand, string comparator): Trigger(comparand, comparator) {
        name="higher_than";
    };
    series_b compute(const int &len, series_d &comparand){};
    series_b compute(const int &len, series_d &comparand, series_d &comparator);
    series_b compute(const int &len, series_d &comparand, double &comparator);
    shared_ptr<Trigger> get_contra() override;
};

class LowerThan : public Trigger {
    LowerThan();
public:
    LowerThan(string comparand, string comparator): Trigger(comparand, comparator) {
        name="lower_than";
    };
    series_b compute(const int &len, series_d &comparand){};
    series_b compute(const int &len, series_d &comparand, series_d &comparator);
    series_b compute(const int &len, series_d &comparand, double &comparator);
    shared_ptr<Trigger> get_contra() override;
};

class CrossesUpward : public Trigger {
    CrossesUpward();
public:
    CrossesUpward(string comparand, string comparator): Trigger(comparand, comparator) {
        name="crosses_upward";
    };
    series_b compute(const int &len, series_d &comparand){};
    series_b compute(const int &len, series_d &comparand, series_d &comparator);
    series_b compute(const int &len, series_d &comparand, double &comparator);
    shared_ptr<Trigger> get_contra() override;
};

class CrossesDownward : public Trigger {
    CrossesDownward();
public:
    CrossesDownward(string comparand, string comparator): Trigger(comparand, comparator) {
        name="crosses_upward";
    };
    series_b compute(const int &len, series_d &comparand){};
    series_b compute(const int &len, series_d &comparand, series_d &comparator);
    series_b compute(const int &len, series_d &comparand, double &comparator);
    shared_ptr<Trigger> get_contra() override;
};

vector<shared_ptr<Trigger>> higherLowerThanTriggers(string comparand, string comparator);
vector<shared_ptr<Trigger>> crossingTriggers(string comparand, string comparator);
vector<shared_ptr<Trigger>> riseFallTriggers(string comparand);
vector<shared_ptr<Trigger>> directionChangeTriggers(string comparand);

std::vector<shared_ptr<Trigger>> operator+(const std::vector<shared_ptr<Trigger>>& v1, const std::vector<shared_ptr<Trigger>>& v2);
std::vector<shared_ptr<Trigger>> operator+(const std::vector<shared_ptr<Trigger>>& v1, const shared_ptr<Trigger>& v2);


#endif //CRYPTONITE_TRIGGER_H
