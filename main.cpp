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

    // Trade Size
    auto tradeSize = configure->add_option_group("Trade Size Configuration");
    auto bidirectionalPolicy = tradeSize->add_option_group("Bidirectional Trade Policy",
                                                           ("Whether to generate bidirectional (long and short) strategies. Default Policy is:" + policyToString(config.tradeSizeGenConfig.bidirectionalTradePolicy)));

    bidirectionalPolicy->require_option(0, 1);
    bidirectionalPolicy->add_flag_callback("--bi-always,--bia",
                                [&](){ config.tradeSizeGenConfig.bidirectionalTradePolicy = Policy::ALWAYS;}, "Always generate bidirectional strategies");
    auto neverBi = bidirectionalPolicy->add_flag_callback("--bi-never,--bin",
                                                          [&](){ config.tradeSizeGenConfig.bidirectionalTradePolicy = Policy::NEVER;}, "Never generate bidirectional strategies");
    bidirectionalPolicy->add_flag_callback("--bi-sometimes,--bis",
                                [&](){ config.tradeSizeGenConfig.bidirectionalTradePolicy = Policy::SOMETIMES;}, "Sometimes generate bidirectional strategies");

    auto fixedSizePolicy = tradeSize->add_option_group("Fixed Trade Size Policy",
                                                           ("Whether generated strategies can use fixed trade size. Default Policy is:" + policyToString(config.tradeSizeGenConfig.fixedTradeSizePolicy)));

    fixedSizePolicy->require_option(0, 1);
    fixedSizePolicy->add_flag_callback("--fixed-always,--fia",
                                           [&](){ config.tradeSizeGenConfig.fixedTradeSizePolicy = Policy::ALWAYS;}, "Always generate strategies that used fixed trade size");
    fixedSizePolicy->add_flag_callback("--fixed-never,--fin",
                                                          [&](){ config.tradeSizeGenConfig.fixedTradeSizePolicy = Policy::NEVER;}, "Never generate strategies that used fixed trade size");
    fixedSizePolicy->add_flag_callback("--fixed-sometimes,--fis",
                                           [&](){ config.tradeSizeGenConfig.fixedTradeSizePolicy = Policy::SOMETIMES;}, "Sometimes generate strategies that used fixed trade size");



    // Take Profit
    auto takeProfit = configure->add_option_group("Take Profit Configuration", "");
    auto tpPolicy = takeProfit->add_option_group("Policy",
                                                 ("Whether to use take-profit when opening Trade. Default Policy is:" + policyToString(config.takeProfitGenConfig.policy)));
    tpPolicy->require_option(0, 1);
    tpPolicy->add_flag_callback("--tp-always,--tpa",
                                [&](){ config.takeProfitGenConfig.policy = Policy::ALWAYS;}, "Always use take-profit");
    auto neverTP = tpPolicy->add_flag_callback("--tp-never,--tpn",
                                               [&](){ config.takeProfitGenConfig.policy = Policy::NEVER;}, "Never use take-profit");
    tpPolicy->add_flag_callback("--tp-sometimes,--tps",
                                [&](){ config.takeProfitGenConfig.policy = Policy::SOMETIMES;}, "Sometimes use take-profit");

    auto tpRange = takeProfit->add_option_group("Range", "Allowed Range of chosen take-profit (EXCLUDES --tp-never)")->excludes(neverTP);
    tpRange->add_option("--tp-min,--tpm", config.takeProfitGenConfig.tpMin, "Minimum take-profit")
           ->ignore_case(false)
           ->check(CLI::Range(0.01, 0.1))
           ->capture_default_str();
    tpRange->add_option("--tp-max,--tpM", config.takeProfitGenConfig.tpMax, "Maximum take-profit")
          ->ignore_case(false)
          ->check(CLI::Range(0.1, 0.2))
          ->capture_default_str();

    //Stop Loss
    auto stopLoss = configure->add_option_group("Stop Loss Configuration", "");
    auto slPolicy = stopLoss->add_option_group("Policy",
                                                 ("Whether to use stop-loss when opening Trade. Default Policy is:" + policyToString(config.stopLossGenConfig.policy)));
    slPolicy->require_option(0, 1);
    slPolicy->add_flag_callback("--sl-always,--sla",
                                [&](){ config.stopLossGenConfig.policy = Policy::ALWAYS;}, "Always use stop-loss");
    auto neverSL = slPolicy->add_flag_callback("--sl-never,--sln",
                                               [&](){ config.stopLossGenConfig.policy = Policy::NEVER;}, "Never use stop-loss");
    slPolicy->add_flag_callback("--sl-sometimes,--sls",
                                [&](){ config.stopLossGenConfig.policy = Policy::SOMETIMES;}, "Sometimes use stop-loss");


    auto slRange = stopLoss->add_option_group("Range", "Allowed Range of chosen stop-loss (EXCLUDES --sl-never)")->excludes(neverSL);
    slRange->add_option("--sl-min,--slm", config.stopLossGenConfig.slMax, "Minimum stop-loss")
            ->ignore_case(false)
            ->check(CLI::Range(0.01, 0.1))
            ->capture_default_str();
    slRange->add_option("--sl-max,--slM", config.stopLossGenConfig.slMin, "Maximum stop-loss")
            ->ignore_case(false)
            ->check(CLI::Range(0.1, 0.2))
            ->capture_default_str();

    auto slType = stopLoss->add_option_group("Type",
                                             ("Type of stop-loss. Default Type is: " + slTypeToString(config.stopLossGenConfig.type) + " (EXCLUDES --sl-never)"))
                          ->excludes(neverSL);

    slType->require_option(0, 1);
    slType->add_flag_callback("--sl-fixed,--slf",
                                [&](){ config.stopLossGenConfig.type = SLType::FIXED;}, "Always use stop-loss");
    slType->add_flag_callback("--sl-trailing,--slt",
                                               [&](){ config.stopLossGenConfig.type = SLType::TRAILING;}, "Never use stop-loss");
    slType->add_flag_callback("--sl-either,--sle",
                              [&](){ config.stopLossGenConfig.type = SLType::EITHER;}, "Sometimes use stop-loss");

    
    // Broker Config
    auto brokerConfig = configure->add_option_group("Broker Configuration");
    brokerConfig->add_option("--commission,-c", config.brokerConfig.commission, "Commission per closed trade")->check(CLI::Range(0.0, 1.0))
                ->capture_default_str();
    brokerConfig->add_option("--slippage, -s", config.brokerConfig.slippage, "Slippage to be applied when opening/closing a trade")->check(CLI::Range(0.0, 1.0))
                ->capture_default_str();

    // Deposit Config
    auto depositConfig = configure->add_option_group("Deposit Configuration");
    depositConfig->add_option("--quote-deposit,-d", config.depositConfig.quoteDeposit, "Opening balance in quote units.")
                  ->check(CLI::PositiveNumber)
                  ->capture_default_str();
    depositConfig->add_option("--max-base-borrow,--mbb", config.depositConfig.maxBaseBorrow,
                              "Specifies amount of base units available to borrow for short-selling.\nNegative Values mean infinite borrowing allowance (Bidirectional Trades only)")
            ->capture_default_str()
            ->excludes(neverBi);

    return config;
}

int main(int argc, char **argv) {
    CLI::App app{"Cryptonite Application is a series of commands that lets you generate, backtest and"
                 "optimise a trading strategy.", "cryptonite"};
//    app.formatter(std::make_shared<MyFormatter>());
    app.get_formatter()->column_width(80);
    app.set_help_all_flag("--help-all", "Expand all help");
    app.require_subcommand(1); // require one subcommand, either configure, generate, optimize
    StrategyGenConfig config = configure(app);
    CLI11_PARSE(app, argc, argv);
//    testIndicators();
    return 0;
}

