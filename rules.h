//
// Created by Abhishek Aggarwal on 26/10/2021.
//

#ifndef CRYPTONITE_RULES_H
#define CRYPTONITE_RULES_H

#include <memory>

using namespace std;

unique_ptr<bool[]> rises(const int &len, const std::unique_ptr<double[]> &series, const std::unique_ptr<double[]> &dummy);
unique_ptr<bool[]> falls(const int &len, const std::unique_ptr<double[]> &series, const std::unique_ptr<double[]> &dummy);
unique_ptr<bool[]> changes_direction_upward(const int &len, const std::unique_ptr<double[]> &series, const std::unique_ptr<double[]> &dummy);
unique_ptr<bool[]> changes_direction_downward(const int &len, const std::unique_ptr<double[]> &series, const std::unique_ptr<double[]> &dummy);
unique_ptr<bool[]> higher_than(const int &len, const std::unique_ptr<double[]> &seriesA, const std::unique_ptr<double[]> &seriesB);
unique_ptr<bool[]> lower_than(const int &len, const std::unique_ptr<double[]> &seriesA, const std::unique_ptr<double[]> &seriesB);
unique_ptr<bool[]> crosses_upward(const int &len, const std::unique_ptr<double[]> &seriesA, const std::unique_ptr<double[]> &seriesB);
unique_ptr<bool[]> crosses_downward(const int &len, const std::unique_ptr<double[]> &seriesA, const std::unique_ptr<double[]> &seriesB);


//@clean_args def falls(series, _):
//return series < np_shift(series, 1, np.nan)
//
//
//@clean_args
//        def higher_than(series1, series2):
//return series1 > series2
//
//
//@clean_args
//        def lower_than(series1, series2):
//return series1 < series2
//
//
//@clean_args
//        def crosses_upward(series1, series2):
//return np.logical_and(series1 > series2, np_shift(series1, 1, np.nan) < np_shift(series2, 1, np.nan))
//
//
//@clean_args
//        def crosses_downward(series1, series2):
//return np.logical_and(series1 < series2, np_shift(series1, 1, np.nan) > np_shift(series2, 1, np.nan))
//
//
//@clean_args
//        def changes_direction_upward(series, _):
//one_shift = np_shift(series, 1, np.nan)
//return np.logical_and(series > one_shift, one_shift < np_shift(series, 2, np.nan))
//
//
//@clean_args
//        def changes_direction_downward(series, _):
//one_shift = np_shift(series, 1, np.nan)
//return np.logical_and(series[-1] < one_shift, one_shift > np_shift(series, 2, np.nan))

//class rules {
//
//};


#endif //CRYPTONITE_RULES_H
