//
// Created by Abhishek Aggarwal on 01/11/2021.
//

#ifndef CRYPTONITE_CONSTANTS_H
#define CRYPTONITE_CONSTANTS_H

#include <limits>
#include <string>

extern double sample_std;
extern double sample_offset;
extern double dMin;
extern double dMax;
extern double dNaN;


constexpr unsigned int switchHash(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (switchHash(str, h+1) * 33) ^ str[h];
}

#endif //CRYPTONITE_CONSTANTS_H
