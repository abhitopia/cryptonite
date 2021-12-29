#include <iostream>
#include <omp.h>
#include "src/dataset.h"
#include "src/indicator.h"
#include "src/strategy.h"
#include "src/backtest.h"
#include "src/datastore.h"
#include "include/progressbar.hpp"
#include "include/CLI11.hpp"
#include "cli/app.h"
#include <cpr/cpr.h>



//using namespace std;
using namespace std::chrono;


void testIndicators(){
    Dataset dataset = Dataset::from_csv("tests/ETHGBP_5m.csv");

    std::cout << "Interval: " << dataset.intervalSeconds() << std::endl;
    std::cout << "Duration: " << dataset.durationDays() << std::endl;
    setup(dataset);
    StrategyGenConfig config;
    auto j = config.toJson();
    std::cout << std::setw(4) << j << std::endl;
    double start_time = omp_get_wtime();

    int numIterations = 100;
    progressbar bar(numIterations);
    #pragma omp parallel for default(none) shared(dataset, config, numIterations, bar)
    for(int i=0; i<numIterations;i++){
        Strategy strategy = Strategy::generate(config);
        Backtest backtester;
        backtester(strategy, dataset);
        #pragma omp critical
        {
            bar.update();
        };
    }
    double time = omp_get_wtime() - start_time;
    std::cout << "\n" << time << std::endl;

}


int main(int argc, char **argv) {
    CryptoniteApp app{};

    cpr::Response r = cpr::Get(cpr::Url{"http://www.httpbin.org/get"});

    auto info = DataSetInfo("ETH", "GBP",  Interval::MINUTE3);
    auto ds = DataStore(info, "datastore");
    return app.run(argc,argv);
}

