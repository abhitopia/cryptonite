//
// Created by Abhishek Aggarwal on 26/10/2021.
//

#ifndef CRYPTONITE_TRIGGER_H
#define CRYPTONITE_TRIGGER_H

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include "../include/json.h"

using json = nlohmann::json;

typedef const std::shared_ptr<double[]> series_d;
typedef std::shared_ptr<bool[]> series_b;


series_b UnimplementedError();

class Trigger {
protected:
    std::string name;
    std::string _comparand;
    std::string _comparator;

public:
    Trigger() = delete;
    Trigger(std::string comparand, std::string comparator): _comparand(std::move(comparand)), _comparator(std::move(comparator)){};
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
public:
    Rises() = delete;
    explicit Rises(std::string comparand): Trigger(std::move(comparand), "") {
        name = "rises";
    };
    series_b compute(const int &len, series_d &comparand) override;
    series_b compute(const int &len, series_d &comparand, series_d &comparator) override{return UnimplementedError();};
    series_b compute(const int &len, series_d &comparand, double &comparator) override{return UnimplementedError();};
    std::shared_ptr<Trigger> get_contra() override;
};

class Falls: public Trigger {
public:
    Falls() = delete;
    explicit Falls(std::string comparand): Trigger(std::move(comparand), "") {
        name="falls";
    };
    series_b compute(const int &len, series_d &comparand) override;
    series_b compute(const int &len, series_d &comparand, series_d &comparator) override{return UnimplementedError();};
    series_b compute(const int &len, series_d &comparand, double &comparator) override{return UnimplementedError();};
    std::shared_ptr<Trigger> get_contra() override;
};

class ChangesDirectionUpward : public Trigger {
public:
    ChangesDirectionUpward() = delete;
    explicit ChangesDirectionUpward(std::string comparand): Trigger(std::move(comparand), "") {
        name="changes_direction_upward";
    };
    series_b compute(const int &len, series_d &comparand) override;
    series_b compute(const int &len, series_d &comparand, series_d &comparator) override{return UnimplementedError();};
    series_b compute(const int &len, series_d &comparand, double &comparator) override{return UnimplementedError();};
    std::shared_ptr<Trigger> get_contra() override;
};

class ChangesDirectionDownward : public Trigger {
public:
    ChangesDirectionDownward() = delete;
    explicit ChangesDirectionDownward(std::string comparand): Trigger(std::move(comparand), "") {
        name="changes_direction_downward";
    };
    series_b compute(const int &len, series_d &comparand) override;
    series_b compute(const int &len, series_d &comparand, series_d &comparator) override{return UnimplementedError();};
    series_b compute(const int &len, series_d &comparand, double &comparator) override{return UnimplementedError();};
    std::shared_ptr<Trigger> get_contra() override;
};

class HigherThan : public Trigger {
public:
    HigherThan() = delete;
    HigherThan(std::string comparand, std::string comparator): Trigger(std::move(comparand), std::move(comparator)) {
        name="higher_than";
    };
    series_b compute(const int &len, series_d &comparand) override{return UnimplementedError();};
    series_b compute(const int &len, series_d &comparand, series_d &comparator) override;
    series_b compute(const int &len, series_d &comparand, double &comparator) override;
    std::shared_ptr<Trigger> get_contra() override;
};

class LowerThan : public Trigger {
public:
    LowerThan() = delete;

    LowerThan(std::string comparand, std::string comparator): Trigger(std::move(comparand), std::move(comparator)) {
        name="lower_than";
    };
    series_b compute(const int &len, series_d &comparand) override{return UnimplementedError();};
    series_b compute(const int &len, series_d &comparand, series_d &comparator) override;
    series_b compute(const int &len, series_d &comparand, double &comparator) override;
    std::shared_ptr<Trigger> get_contra() override;
};

class CrossesUpward : public Trigger {
public:
    CrossesUpward() = delete;
    CrossesUpward(std::string comparand, std::string comparator): Trigger(std::move(comparand), std::move(comparator)) {
        name="crosses_upward";
    };
    series_b compute(const int &len, series_d &comparand) override{return UnimplementedError();};
    series_b compute(const int &len, series_d &comparand, series_d &comparator) override;
    series_b compute(const int &len, series_d &comparand, double &comparator) override;
    std::shared_ptr<Trigger> get_contra() override;
};

class CrossesDownward : public Trigger {
public:
    CrossesDownward() = delete;
    CrossesDownward(std::string comparand, std::string comparator): Trigger(std::move(comparand), std::move(comparator)) {
        name="crosses_upward";
    };
    series_b compute(const int &len, series_d &comparand) override{return UnimplementedError();};
    series_b compute(const int &len, series_d &comparand, series_d &comparator) override;
    series_b compute(const int &len, series_d &comparand, double &comparator) override;
    std::shared_ptr<Trigger> get_contra() override;
};

std::vector<std::shared_ptr<Trigger>> higherLowerThanTriggers(std::string comparand, std::string comparator);
std::vector<std::shared_ptr<Trigger>> crossingTriggers(std::string comparand, std::string comparator);
std::vector<std::shared_ptr<Trigger>> riseFallTriggers(std::string comparand);
std::vector<std::shared_ptr<Trigger>> directionChangeTriggers(std::string comparand);

std::vector<std::shared_ptr<Trigger>> operator+(const std::vector<std::shared_ptr<Trigger>>& v1, const std::vector<std::shared_ptr<Trigger>>& v2);
std::vector<std::shared_ptr<Trigger>> operator+(const std::vector<std::shared_ptr<Trigger>>& v1, const std::shared_ptr<Trigger>& v2);


#endif //CRYPTONITE_TRIGGER_H
