#include <iostream>
#include <omp.h>
#include "dataset.h"
#include "indicator.h"
#include "strategy.h"
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
    Dataset dataset = Dataset::from_csv("tests/ETHGBP_5m.csv");

    cout << "Interval: " << dataset.intervalSeconds() << endl;
    cout << "Duration: " << dataset.durationDays() << endl;
    setup(dataset);
    StrategyGenConfig config;
    auto j = config.toJson();
    std::cout << std::setw(4) << j << std::endl;
    double start_time = omp_get_wtime();

    #pragma omp parallel for default(none) shared(dataset, config)
    for(int i=0; i< 1000; i++){
        Strategy strategy = Strategy::generate(config);
        Backtest backtester;
        backtester(strategy, dataset);
//        backtester.metrics();
    }
    double time = omp_get_wtime() - start_time;
    cout << time << endl;

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