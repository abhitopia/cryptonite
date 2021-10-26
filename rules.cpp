//
// Created by Abhishek Aggarwal on 26/10/2021.
//

#include "rules.h"
#include <vector>
#include <string>
#include <unordered_map>


using namespace std;

unique_ptr<bool[]> rises(const int &len, const std::unique_ptr<double[]> &series, const std::unique_ptr<double[]> &dummy){
    unique_ptr<bool[]> result{new bool[len]};
    result[0] = false;
    for(int i=1; i < len; i++){
        result[i] = series[i] > series[i-1];
    }
    return result;
}

unique_ptr<bool[]> falls(const int &len, const std::unique_ptr<double[]> &series, const std::unique_ptr<double[]> &dummy){
    unique_ptr<bool[]> result{new bool[len]};
    result[0] = false;

    for(int i=1; i < len; i++){
        result[i] = series[i] < series[i-1];
    }
    return result;
}

unique_ptr<bool[]> changes_direction_upward(const int &len, const std::unique_ptr<double[]> &series, const std::unique_ptr<double[]> &dummy){
    unique_ptr<bool[]> result{new bool[len]};
    result[0] = false;
    result[1] = false;

    for(int i=2; i < len; i++){
        result[i] = series[i] > series[i-1] && series[i-1] < series[i-2];
    }
    return result;
}

unique_ptr<bool[]> changes_direction_downward(const int &len, const std::unique_ptr<double[]> &series, const std::unique_ptr<double[]> &dummy){
    unique_ptr<bool[]> result{new bool[len]};
    result[0] = false;
    result[1] = false;

    for(int i=2; i < len; i++){
        result[i] = series[i] < series[i-1] && series[i-1] > series[i-2];
    }
    return result;
}

unique_ptr<bool[]> higher_than(const int &len, const std::unique_ptr<double[]> &seriesA, const std::unique_ptr<double[]> &seriesB){
    unique_ptr<bool[]> result{new bool[len]};

    for(int i=0; i < len; i++){
        result[i] = seriesA[i] > seriesB[i];
    }
    return result;
}

unique_ptr<bool[]> lower_than(const int &len, const std::unique_ptr<double[]> &seriesA, const std::unique_ptr<double[]> &seriesB){
    unique_ptr<bool[]> result{new bool[len]};

    for(int i=0; i < len; i++){
        result[i] = seriesA[i] < seriesB[i];
    }
    return result;
}

unique_ptr<bool[]> crosses_upward(const int &len, const std::unique_ptr<double[]> &seriesA, const std::unique_ptr<double[]> &seriesB){
    unique_ptr<bool[]> result{new bool[len]};
    result[0] = false;
    for(int i=1; i < len; i++){
        result[i] = seriesA[i] > seriesB[i] && seriesA[i-1] < seriesB[i-1];
    }
    return result;
}


unique_ptr<bool[]> crosses_downward(const int &len, const unique_ptr<double[]> &seriesA, const unique_ptr<double[]> &seriesB) {
    unique_ptr<bool[]> result{new bool[len]};
    result[0] = false;
    for(int i=1; i < len; i++){
        result[i] = seriesA[i] < seriesB[i] && seriesA[i-1] > seriesB[i-1];
    }
    return result;
}

typedef unique_ptr<bool[]> (*rule_func)(const int &, const unique_ptr<double[]> &, const unique_ptr<double[]> &);

std::unordered_map<rule_func, string> FUNC2NAME = {
        {&higher_than, "higher_than"},
        {&lower_than, "lower_than"},
        {&rises, "rises"},
        {&falls, "falls"},
        {&crosses_upward, "crosses_upward"},
        {&crosses_downward, "crosses_downward"},
        {&changes_direction_upward, "changes_direction_upward"},
        {&changes_direction_downward, "changes_direction_upward"}
};

std::unordered_map<rule_func, rule_func> FUNC2CONTRAFUNC = {
        {&higher_than, &lower_than},
        {&lower_than, &higher_than},
        {&rises, &falls},
        {&falls, &rises},
        {&crosses_upward, &crosses_downward},
        {&crosses_downward, &crosses_upward},
        {&changes_direction_upward, &changes_direction_downward},
        {&changes_direction_downward, &changes_direction_upward}
};

struct Rule{
    string name;
    rule_func func;
    string arg1;
    string arg2;
};

Rule& get_contra_rule(Rule& rule){
    rule_func contra_func = FUNC2CONTRAFUNC[rule.func];
    Rule contra_rule = {
            FUNC2NAME[contra_func],
            contra_func,
            rule.arg1,
            rule.arg2
    };
    return contra_rule;
}

vector<Rule>& higher_lower_than_rules(string arg1, string arg2){
    Rule rule = {
            FUNC2NAME[&higher_than],
            &higher_than,
            arg1,
            arg2};
    vector<Rule> result = {rule, get_contra_rule(rule)};
    return result;
}

vector<Rule>& rise_fall_rules(string arg1, string arg2){
    Rule rule = {
            FUNC2NAME[&rises],
            &rises,
            arg1,
            arg2};
    vector<Rule> result = {rule, get_contra_rule(rule)};
    return result;
}

vector<Rule>& crossing_rules(string arg1, string arg2){
    Rule rule = {
            FUNC2NAME[&crosses_upward],
            &crosses_upward,
            arg1,
            arg2};
    vector<Rule> result = {rule, get_contra_rule(rule)};
    return result;
}

vector<Rule>& direction_change_rules(string arg1, string arg2){
    Rule rule = {
            FUNC2NAME[&changes_direction_upward],
            &changes_direction_upward,
            arg1,
            arg2};
    vector<Rule> result = {rule, get_contra_rule(rule)};
    return result;
}

std::vector<Rule>& operator+(const std::vector<Rule>& v1, const std::vector<Rule>& v2){
    std::vector<Rule> vr(std::begin(v1), std::end(v1));
    vr.insert(std::end(vr), std::begin(v2), std::end(v2));
    return vr;
}

//vector<Rule> result = direction_change_rules("a", "b") + direction_change_rules("a", "b");