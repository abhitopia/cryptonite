#include <iostream>
#include <omp.h>
#include "src/dataset.h"
#include "src/indicator.h"
#include "src/strategy.h"
#include "src/backtest.h"
#include "include/progressbar.hpp"
#include "include/CLI11.hpp"

using namespace std;
using namespace std::chrono;


void testIndicators(){
    Dataset dataset = Dataset::from_csv("tests/ETHGBP_5m.csv");

    cout << "Interval: " << dataset.intervalSeconds() << endl;
    cout << "Duration: " << dataset.durationDays() << endl;
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
    cout << "\n" << time << endl;

}

StrategyGenConfig configure(CLI::App& app){
    StrategyGenConfig config;
    CLI::App* configure = app.add_subcommand("configure", "Configure Generation of Strategies");

    configure->add_flag("--tp-sometime{sometime},--tp-always{always}", config.takeProfitGenConfig.policy, "TakeProfitPolicy")
             ->default_val(config.takeProfitGenConfig.policy)
             ->default_str(policyToString(config.takeProfitGenConfig.policy));
    return config;
}

int main(int argc, char **argv) {
    CLI::App app{"Cryptonite Application is a series of commands that lets you generate, backtest and"
                 "optimise a trading strategy.", "cryptonite"};
    app.set_help_all_flag();
    app.require_subcommand(1); // require one subcommand, either configure, generate, optimize
    StrategyGenConfig config = configure(app);

    CLI11_PARSE(app, argc, argv);
//    testIndicators();
    return 0;
}


//enum class Level : int { High, Medium, Low };
//
//int main(int argc, char **argv) {
//    CLI::App app;
//
//    Level level{Level::Low};
//    // specify string->value mappings
//    std::map<std::string, Level> map{{"high",   Level::High},
//                                     {"medium", Level::Medium},
//                                     {"low",    Level::Low}};
//    // CheckedTransformer translates and checks whether the results are either in one of the strings or in one of the
//    // translations already
//    app.add_flag("-l{high},--level{high}, -m{medium}", level, "Level settings")
//            ->required()
//            ->transform(CLI::CheckedTransformer(map, CLI::ignore_case));
//
//    CLI11_PARSE(app, argc, argv);
//
//    // CLI11's built in enum streaming can be used outside CLI11 like this:
//    using CLI::enums::operator<<;
//    std::cout << "Enum received: " << level << std::endl;
//
//    return 0;
//}