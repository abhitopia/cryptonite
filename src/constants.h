//
// Created by Abhishek Aggarwal on 01/11/2021.
//

#ifndef CRYPTONITE_CONSTANTS_H
#define CRYPTONITE_CONSTANTS_H

#include <limits>
#include <string>
#include <vector>
#include "random.h"

extern double sample_std;
extern double sample_offset;
extern double dMin;
extern double dMax;
extern double dNaN;
extern std::vector<std::string> ADJECTIVES;
extern std::vector<std::string> SURNAMES;
extern bool MULTITHREADED;


constexpr unsigned int switchHash(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (switchHash(str, h+1) * 33) ^ str[h];
}


std::string get_random_name();

#endif //CRYPTONITE_CONSTANTS_H
