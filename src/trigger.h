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

typedef const std::shared_ptr<double[]> series_d;
typedef std::shared_ptr<bool[]> series_b;


class Trigger {
protected:
    std::string name = "";
    std::string _comparand = "";
    std::string _comparator = "";

public:
    Trigger() = delete;
    Trigger(std::string comparand, std::string comparator): _comparand(comparand), _comparator(comparator){};
    virtual series_b compute(const int &len, series_d &comparand) = 0;
    virtual series_b compute(const int &len, series_d &comparand, series_d &comparator) = 0;
    virtual series_b compute(const int &len, series_d &comparand, double &comparator) = 0;
    virtual std::shared_ptr<Trigger> get_contra(){return nullptr;};
    bool has_level();
    std::string getComparand();
    std::string getComparator();
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
    Rises(std::string comparand): Trigger(comparand, "") {
        name = "rises";
    };
    series_b compute(const int &len, series_d &comparand) override;
    series_b compute(const int &len, series_d &comparand, series_d &comparator){};
    series_b compute(const int &len, series_d &comparand, double &comparator){};
    std::shared_ptr<Trigger> get_contra() override;
};

class Falls: public Trigger {
    Falls();
public:
    Falls(std::string comparand): Trigger(comparand, "") {
        name="falls";
    };
    series_b compute(const int &len, series_d &comparand);
    series_b compute(const int &len, series_d &comparand, series_d &comparator){};
    series_b compute(const int &len, series_d &comparand, double &comparator){};
    std::shared_ptr<Trigger> get_contra() override;
};

class ChangesDirectionUpward : public Trigger {
    ChangesDirectionUpward();
public:
    ChangesDirectionUpward(std::string comparand): Trigger(comparand, "") {
        name="changes_direction_upward";
    };
    series_b compute(const int &len, series_d &comparand);
    series_b compute(const int &len, series_d &comparand, series_d &comparator){};
    series_b compute(const int &len, series_d &comparand, double &comparator){};
    std::shared_ptr<Trigger> get_contra() override;
};

class ChangesDirectionDownward : public Trigger {
    ChangesDirectionDownward();
public:
    ChangesDirectionDownward(std::string comparand): Trigger(comparand, "") {
        name="changes_direction_downward";
    };
    series_b compute(const int &len, series_d &comparand);
    series_b compute(const int &len, series_d &comparand, series_d &comparator){};
    series_b compute(const int &len, series_d &comparand, double &comparator){};
    std::shared_ptr<Trigger> get_contra() override;
};

class HigherThan : public Trigger {
    HigherThan();
public:
    HigherThan(std::string comparand, std::string comparator): Trigger(comparand, comparator) {
        name="higher_than";
    };
    series_b compute(const int &len, series_d &comparand){};
    series_b compute(const int &len, series_d &comparand, series_d &comparator);
    series_b compute(const int &len, series_d &comparand, double &comparator);
    std::shared_ptr<Trigger> get_contra() override;
};

class LowerThan : public Trigger {
    LowerThan();
public:
    LowerThan(std::string comparand, std::string comparator): Trigger(comparand, comparator) {
        name="lower_than";
    };
    series_b compute(const int &len, series_d &comparand){};
    series_b compute(const int &len, series_d &comparand, series_d &comparator);
    series_b compute(const int &len, series_d &comparand, double &comparator);
    std::shared_ptr<Trigger> get_contra() override;
};

class CrossesUpward : public Trigger {
    CrossesUpward();
public:
    CrossesUpward(std::string comparand, std::string comparator): Trigger(comparand, comparator) {
        name="crosses_upward";
    };
    series_b compute(const int &len, series_d &comparand){};
    series_b compute(const int &len, series_d &comparand, series_d &comparator);
    series_b compute(const int &len, series_d &comparand, double &comparator);
    std::shared_ptr<Trigger> get_contra() override;
};

class CrossesDownward : public Trigger {
    CrossesDownward();
public:
    CrossesDownward(std::string comparand, std::string comparator): Trigger(comparand, comparator) {
        name="crosses_upward";
    };
    series_b compute(const int &len, series_d &comparand){};
    series_b compute(const int &len, series_d &comparand, series_d &comparator);
    series_b compute(const int &len, series_d &comparand, double &comparator);
    std::shared_ptr<Trigger> get_contra() override;
};

std::vector<std::shared_ptr<Trigger>> higherLowerThanTriggers(std::string comparand, std::string comparator);
std::vector<std::shared_ptr<Trigger>> crossingTriggers(std::string comparand, std::string comparator);
std::vector<std::shared_ptr<Trigger>> riseFallTriggers(std::string comparand);
std::vector<std::shared_ptr<Trigger>> directionChangeTriggers(std::string comparand);

std::vector<std::shared_ptr<Trigger>> operator+(const std::vector<std::shared_ptr<Trigger>>& v1, const std::vector<std::shared_ptr<Trigger>>& v2);
std::vector<std::shared_ptr<Trigger>> operator+(const std::vector<std::shared_ptr<Trigger>>& v1, const std::shared_ptr<Trigger>& v2);


#endif //CRYPTONITE_TRIGGER_H
