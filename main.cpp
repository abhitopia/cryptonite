#include <iostream>
#include <omp.h>
#include "dataset.h"
#include "indicator.h"
#include <chrono>
#include "function.h"
#include "strategy.h"
#include "random.h"
#include "backtest.h"

using namespace std;
using namespace std::chrono;


void openMP() {
    double start_time = omp_get_wtime();
    int i;
    int threadID = 0;
    #pragma omp parallel for private(i, threadID)
    for(i = 0; i < 20; i++ )
    {
        threadID = omp_get_thread_num();

        #pragma omp critical
        {
            printf("Thread %d reporting\n", threadID);
        }
    }

    double time = omp_get_wtime() - start_time;
    cout << time << endl;

}

void testIndicators(){
//    int num_indicators = Indicators.size();
    Dataset dataset = Dataset::from_csv("tests/ETHGBP_5m.csv");
    setup(dataset);
    StrategyGenConfig config;
    auto j = config.toJson();
    std::cout << std::setw(4) << j << std::endl;


    Strategy strategy = Strategy::generate(config);

//    backtest(strategy, dataset);
//    int i = 1;

//    bool test = dNaN == dNaN;
//    cout << test;
//    setup(dataset);
//
//    shared_ptr<Indicator> indicator{nullptr};
//    IndicatorConfig config();
//    for(int i=0; i<num_indicators; i++){
//        for (int j=0; j< 1000; j++){
//            cout << "Indicator Number: " << i;
//            indicator = Indicators[i];
//            config = indicator->generate_config(0.5);
//            indicator->compute(dataset, config);
//        }
//    }
}


int main() {
//    RandomGenerator gen{};
//    _Random->seed(42);
    testIndicators();

//#pragma omp parallel for
//    for(int i=0; i < 100; i++){
//        cout << "Generated: " << randn() << endl;
//    }

//    Dataset dataset = Dataset::from_csv("tests/ETHGBP_5m.csv");

//    auto result = CIndicator::abs(dataset.num_bars, {dataset.typical});

//    Alligator ind{};
//    IndicatorConfig config = ind.generate_config(1.0);
//    for(int i=0; i< 100; i++) {
//        cout << cryptonite::randint(0, 2) << endl;
//    }
//    auto result1 = ind.compute(dataset, config);
//    cout << result1["value"][0] << endl;
//    unique_ptr<bool[]> result{std::move(rises(dataset.num_bars, dataset.open, dataset.open))};

    return 0;
}


/*
 * Generated: In Thread 0
0.796543
Generated: In Thread 0
0.183435
Generated: In Thread 0
0.779691
Generated: In Thread 0
0.59685
Generated: In Thread 0
0.445833
Generated: In Thread 0
0.0999749
Generated: In Thread 0
0.459249
Generated: In Thread 0
0.333709
Generated: In Thread 0
0.142867
Generated: In Thread 0
0.650888
8The number of members in trigger 8
Process finished with exit code 0

 */