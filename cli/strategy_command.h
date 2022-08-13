//
// Created by Abhishek Aggarwal on 31/12/2021.
//

#ifndef CLI_STRATEGY_H
#define CLI_STRATEGY_H

#include <unordered_map>
#include <limits>
#include "../include/CLI11.hpp"
#include "../src/config.h"
#include "../src/dataset.h"
#include "../src/indicator.h"
#include "../src/backtest.h"
#include "command.h"
#include "../src/top_n_container.h"
#include "../src/genetic_algorithm.hpp"
#include "../src/dna.h"


struct GeneratedStrategy {
    std::string name{};
    int version{};
    json metrics{};
    json strategy{};
    json dataset{};
    std::string createdAt{};
    GeneratedStrategy(){};

    static GeneratedStrategy fromBacktest(std::string name, int version, const Backtest &backtest) {
        auto backtestJson = backtest.toJson();
        backtestJson["createdAt"] = date::format("%F %T", std::chrono::system_clock::now());
        return fromJson(name, version, backtestJson);
    }

    json toJson() const {
        json j;
        j["metrics"] = metrics;
        j["strategy"] = strategy;
        j["dataset"] = dataset;
        j["createdAt"] = createdAt;
        return j;
    }

    static GeneratedStrategy fromJson(std::string name, int version, json j){
        auto generatedStrategy = GeneratedStrategy{};
        generatedStrategy.name = name;
        generatedStrategy.version = version;
        generatedStrategy.metrics = j["metrics"];
        generatedStrategy.strategy = j["strategy"];
        generatedStrategy.dataset = j["dataset"];
        generatedStrategy.createdAt = j["createdAt"].get<std::string>();
        return generatedStrategy;
    }

    double metric() const {
        return metrics["CAGROverAvgDrawDown"].get<double>() * metrics["numTrades"].get<double>();
    }

    bool operator>(const GeneratedStrategy& other) const {
        return this->metric() > other.metric();
    }

    bool operator<(const GeneratedStrategy& other) const {
        return this->metric() < other.metric();
    }
};




struct StrategyCommand: CryptoniteCommand {
    json jsonDB{};
    DataStore datastore{};

    void setup(CLI::App& cli_app) override {
        app = &cli_app;
        command = app->add_subcommand("strategy", "Generate, optimize and analyse strategies");
        command->require_subcommand(1);

        auto generateCommand = command->add_subcommand("generate", "Generate strategies.");
        generateCommand->add_option("--config,-c,config", "Name of the configuration.")
                ->check(CLI::TypeValidator<std::string>())
                ->required();

        generateCommand->add_option("--version,-v,version", "Version of the configuration. Default version used if not specified")
                ->check(CLI::PositiveNumber);

        auto listCommand = addSubcommand("list", "List generated strategies.", -1);
        auto deleteCommand = addSubcommand("delete", "Delete generated strategies.", -1);
        auto optimizeCommand = addSubcommand("optimize", "Optimize generated strategies.", -1);
        auto analyzeCommand = addSubcommand("analyze", "Optimize generated strategies.", -1);

        optimizeCommand->add_flag("--verbose,-V", "Print during optimization");
        optimizeCommand->add_option("--pop-size,-p", "Size of the population.")
                ->default_val(500)
                ->check(CLI::Range(100, 2000, "[100-2000] "));
        optimizeCommand->add_option("--frac-reproduce,-f", "Fraction of elite population that can reproduce.")
                ->default_val(0.4)
                ->check(CLI::Range(0.1, 1.0, "[0.1-1.0] "));
        optimizeCommand->add_option("--cross-prob,-c", "Recombination probability.")
                ->default_val(0.9)
                ->check(CLI::Range(0.1, 1.0, "[0.1-1.0] "));

        analyzeCommand->add_option("--max-noise,-m,max-noise", "Maximum noise level")
                ->default_val(0.01)
                ->check(CLI::Range(0.005, 0.05, "[0.005- 0.05] "));

        analyzeCommand->add_option("--num-samples,-s,num-samples", "Number of samples in each step")
                      ->default_val(100)
                      ->check(CLI::Range(100, 1000, "[100, 1000] "));

        analyzeCommand->add_option("--num-steps,-d,num-steps", "Number of steps")
                      ->default_val(10)
                       ->check(CLI::Range(5, 20, "[5-20] "));


        command->fallthrough(true);
    }

    void parse() override {
        datastore = DataStore{app->get_option("--datastore")->as<std::string>()};
        if(command->count() > 0){
            jsonDB = JsonFileHandler::read(app->get_option("--database")->as<std::string>(), true);
            if(!jsonDB.contains("strategies"))
                jsonDB["strategies"] = json::object({});

            CLI::App* subcommand = command->get_subcommands()[0];
            if(subcommand->check_name("generate")){
                std::string name = subcommand->get_option("--config")->as<std::string>();
                int version =  subcommand->get_option("--version")->count() > 0 ? subcommand->get_option("--version")->as<int>() : -1;
                if(!hasConfig(name, version)){
                    std::string versionUsed = version < 0 ? "default" : std::to_string(version);
                    std::cout << "Configuration `" << name << "` (version: " << versionUsed << ") does not exist!" << std::endl;
                    return;
                }

                auto config = getConfig(name, version);
                showConfig(name, version);
                generate(config);
            } else {
                json filter;
                filter["name"] = subcommand->get_option("--name")->as<std::string>();
                filter["quoteAsset"] = subcommand->get_option("--quoteAsset")->as<std::string>();
                filter["baseAsset"] = subcommand->get_option("--baseAsset")->as<std::string>();
                filter["interval"] = subcommand->get_option("--interval")->as<std::string>();
                filter["version"] = subcommand->get_option("--version")->as<int>();
                auto generatedStrategies = loadGeneratedStrategies(filter);
                if(generatedStrategies.size() == 0){
                    std::cout << "No strategies match the criteria!!" << std::endl;
                    return;
                }
                if(subcommand->check_name("list")){
                    showTopN(generatedStrategies);
                } else if(subcommand->check_name("delete")){
                    showTopN(generatedStrategies, "Are you sure you want to delete these?");
                    std::cout << "Press `d` to delete listed strategies: ";
                    char answer = std::cin.get();
                    if(answer == 'd') {
                        deleteGenerateStrategies(generatedStrategies);
                    }
                } else if(subcommand->check_name("optimize")){
                    showTopN(generatedStrategies, "Following strategies will be optimized");
                    int popSize = subcommand->get_option("--pop-size")->as<int>();
                    double fractionReproduce = subcommand->get_option("--frac-reproduce")->as<double>();
                    double crossProb = subcommand->get_option("--cross-prob")->as<double>();
                    bool verbose = subcommand->get_option("--verbose")->as<bool>();
                    optimize(generatedStrategies, popSize, fractionReproduce, crossProb, verbose);
                } else if(subcommand->check_name("analyze")){
                    showTopN(generatedStrategies, "Following strategies will be analyzed");
                    double noise = subcommand->get_option("--max-noise")->as<double>();
                    int numSamples = subcommand->get_option("--num-samples")->as<int>();
                    int numSteps = subcommand->get_option("--num-steps")->as<int>();
                    analyze(generatedStrategies, noise, numSamples, numSteps);
                }
            }
        }
    }


private:
    CLI::App* addSubcommand(string name, string info, int versionDefault){
        auto subcommand = command->add_subcommand(name, info);
        subcommand->add_option("--name,-n", "Filter strategy by name");
        subcommand->add_option("--baseAsset,-b,baseAsset", "Filter strategies by base asset")->excludes("--name");
        subcommand->add_option("--quoteAsset,-q,quoteAsset", "Filter strategies by quote asset")->excludes("--name");
        subcommand->add_option("--interval,-i,interval", "Filter strategies by interval")->excludes("--name");
        subcommand->add_option("--version,-v", "Filter strategies by version. `0` for unoptimized strategies.")
                ->default_val(versionDefault)
                ->check(CLI::Range(-1, 100, "[0, Inf] "));
        return subcommand;
    }

    void analyze(std::vector<GeneratedStrategy> strategies, double noise, int numSamples, int numSteps){
        for(auto generatedStrategy: strategies){
            std::cout << "--------------------------------" << generatedStrategy.name << std::endl;
            for(int s=0; s < numSteps + 1; s++){
                double noise_s = (s*noise)/numSteps;
                DataSetConfig dataSetConfig = DataSetConfig::fromJson(generatedStrategy.dataset);
                vector<Metrics> metrics(numSamples);
                for(int n=0; n < numSamples; n++){
                    auto strategy = Strategy::fromJson(generatedStrategy.strategy);
                    auto backtest = doBackTest(strategy, datastore.getDataset(dataSetConfig, noise_s));
                    metrics[n] = backtest.metrics;
                }
                std::sort(metrics.begin(), metrics.end(), std::greater<Metrics>());
                std::cout << noise_s << ": " << metrics[0].metric() << " : " << metrics[numSamples-1].metric() << std::endl;
            }
        }
    }

    void optimize(std::vector<GeneratedStrategy> strategies, size_t populationSize, double fractionReproduce, double recombinationProb, bool verbose=false){
        int i = 0;
        for(auto generatedStrategy: strategies){
            i += 1;
            std::string appendix = std::to_string(i) + "/" + std::to_string(strategies.size());
            showTopN(std::vector<GeneratedStrategy>{generatedStrategy}, "Optimizing Strategy : " + appendix);
            DataSetConfig dataSetConfig = DataSetConfig::fromJson(generatedStrategy.dataset);
            // TODO(abhi) this calls Indicator::setup every single time. It might be more efficient to cache those value in datastore manager
            auto dataset = std::make_shared<Dataset>(datastore.getDataset(dataSetConfig));
//            Indicator::setup(*dataset.get()); // This is not needed since we use existing value
            StrategyDNA strategyDna{dataset, generatedStrategy.strategy};
            GeneticAlgorithm<StrategyDNA> ga{strategyDna, populationSize, fractionReproduce, recombinationProb , -1.0};
            ga.optimize(5, verbose);
            Backtest backtest = ga.getBestDNA().getBacktest();
            auto optimizedStrategy = saveGeneratedStrategy(backtest, generatedStrategy.name, generatedStrategy.version + 1);
            showTopN(std::vector<GeneratedStrategy>{optimizedStrategy}, "Optimized Strategy!");
        }
    }



    void deleteGenerateStrategies(std::vector<GeneratedStrategy> strategies){
        for(auto strategy: strategies){
            jsonDB["strategies"].erase(strategy.name);
        }

        JsonFileHandler::write(app->get_option("--database")->as<std::string>(), jsonDB);
    }

    std::vector<GeneratedStrategy> loadGeneratedStrategies(json filter){

        std::vector<GeneratedStrategy> generatedStrategies{};
        auto topN = TopNContainer<GeneratedStrategy>{1000};

        if(not jsonDB.contains("strategies") ){
            return generatedStrategies;
        };

        auto strategyName = filter["name"].get<std::string>();
        auto quoteAsset = filter["quoteAsset"].get<std::string>();
        auto baseAsset = filter["baseAsset"].get<std::string>();
        auto interval = filter["interval"].get<std::string>();
        auto version = filter["version"].get<int>();

        for(auto& [name, value]: jsonDB["strategies"].items()){
            if(strategyName.length() > 0 and strategyName != name){
                continue;
            }

            int defaultVersion = value["defaultVersion"].get<int>();

            if(version >= 0 and defaultVersion != version){
                continue;
            }

            json strategy = value["versions"][defaultVersion];
            if(strategyName.length() > 0 and strategyName != name){
                continue;
            }

            if(quoteAsset.length() > 0 and quoteAsset != strategy["dataset"]["quoteAsset"].get<std::string>()){
                continue;
            }
            if(baseAsset.length() > 0 and baseAsset != strategy["dataset"]["baseAsset"].get<std::string>()){
                continue;
            }
            if(interval.length() > 0 and interval != strategy["dataset"]["interval"].get<std::string>()){
                continue;
            }

            generatedStrategies.push_back(GeneratedStrategy::fromJson(name, defaultVersion, strategy));
        }

        std::sort(generatedStrategies.begin(), generatedStrategies.end(), std::greater<GeneratedStrategy>());
        return generatedStrategies;
    }


    bool hasConfig(std::string name, int version=-1){
        if(not jsonDB.contains("configs") || not jsonDB["configs"].contains(name)){
            return false;
        };

        json metaConfig = jsonDB["configs"][name];
        if(version <= 0 && !metaConfig["versions"].empty()){
            return true;
        } else if(metaConfig["versions"].contains(std::to_string(version))){
            return true;
        }
        return false;
    }

    json getConfig(std::string name, int version=-1) {
        json metaConfig = jsonDB["configs"][name];
        version = version <= 0 ? getDefaultVersion(name) : version;
        return metaConfig["versions"][std::to_string(version)]["content"];
    }

    int getDefaultVersion(std::string name){
        return jsonDB["configs"][name]["defaultVersion"].get<int>();
    }

    void showConfig(std::string name, int version){
        fort::char_table table;
        table.set_cell_text_align(fort::text_align::center);
        table.set_border_style(FT_BOLD_STYLE);
        table.row(0).set_cell_bg_color(fort::color::black);
        table.row(1).set_cell_bg_color(fort::color::black);

        int defaultVersion = getDefaultVersion(name);
        version = version <= 0 ? defaultVersion : version;
        table[0][0].set_cell_span(4);
        table <<  fort::header  << "Configuration: " + name << fort::endr;
        table << fort::header << "Version" << "Parent Version" << "Created On" << "Content" << fort::endr;
        json& metaConfig = jsonDB["configs"][name];
        for(auto& [configVersion, config]: metaConfig["versions"].items()){
            if(version == atoi(configVersion.c_str())){
                std::string versionStr = configVersion;
                if(atoi(configVersion.c_str()) == defaultVersion){
                    versionStr += " (default)";
                }
                table << versionStr << config["parentVersion"] << config["createdAt"].get<std::string>() << std::setw(2) << config["content"] << fort::endr << fort::separator;
            }
        }
        std::cout << table.to_string() << std::endl;
    }

    StrategyGenConfig getStrategyConfig(json config){
        auto defaultConfig = StrategyGenConfig{}.toJson();

        //acceptanceConfig
        defaultConfig["acceptanceConfig"]["minNumTrades"] = atoi(config["min-num-trades"].get<std::string>().c_str());


        // brokerConfig
        defaultConfig["brokerConfig"]["commission"] = atof(config["commission"].get<std::string>().c_str());
        defaultConfig["brokerConfig"]["slippage"] = atof(config["slippage"].get<std::string>().c_str());

        // dataSetConfig
        defaultConfig["dataSetConfig"]["baseAsset"] = config["base-asset"].get<std::string>();
        defaultConfig["dataSetConfig"]["quoteAsset"] = config["quote-asset"].get<std::string>();
        defaultConfig["dataSetConfig"]["interval"] = config["interval"].get<std::string>();
        defaultConfig["dataSetConfig"]["numBars"] = atoi(config["num-bars"].get<std::string>().c_str());

        // depositConfig
        defaultConfig["depositConfig"]["quoteDeposit"] = atof(config["quote-deposit"].get<std::string>().c_str());
        defaultConfig["depositConfig"]["maxBaseBorrow"] = atof(config["max-base-borrow"].get<std::string>().c_str());

        // rulesGenConfig
        defaultConfig["rulesGenConfig"]["explorationProb"] = atof(config["exploration-prob"].get<std::string>().c_str());
        defaultConfig["rulesGenConfig"]["numMaxEntryRules"] = atoi(config["max-entry-rules"].get<std::string>().c_str());
        defaultConfig["rulesGenConfig"]["numMaxExitRules"] = atoi(config["max-exit-rules"].get<std::string>().c_str());

        // stopLossGenConfig
        defaultConfig["stopLossGenConfig"]["policy"] = config["sl-policy"].get<std::string>();
        defaultConfig["stopLossGenConfig"]["slMax"] = atof(config["sl-max"].get<std::string>().c_str());
        defaultConfig["stopLossGenConfig"]["slMin"] = atof(config["sl-min"].get<std::string>().c_str());
        defaultConfig["stopLossGenConfig"]["type"] = config["sl-type"].get<std::string>();

        // takeProfitGenConfig
        defaultConfig["takeProfitGenConfig"]["policy"] = config["tp-policy"].get<std::string>();
        defaultConfig["takeProfitGenConfig"]["tpMax"] = atof(config["tp-max"].get<std::string>().c_str());
        defaultConfig["takeProfitGenConfig"]["tpMin"] = atof(config["tp-min"].get<std::string>().c_str());

        // tradeSizeGenConfig
        defaultConfig["tradeSizeGenConfig"]["bidirectionalTradePolicy"] = config["bidirectional-policy"].get<std::string>();
        defaultConfig["tradeSizeGenConfig"]["fixedTradeSizePolicy"] = config["fixed-size-policy"].get<std::string>();

        return StrategyGenConfig::fromJson(defaultConfig);
    }


    void generate(json config){
        auto strategyGenConfig = getStrategyConfig(config);
        auto dataset = datastore.getDataset(strategyGenConfig.dataSetConfig);
        Indicator::setup(dataset);  // This only needs to be done for generation
        double start_time = omp_get_wtime();
        auto topN = TopNContainer<GeneratedStrategy>{10};
        unsigned long numEvaluated = 0;
        #pragma omp parallel for default(none) shared(numEvaluated, start_time, topN, dataset, strategyGenConfig) if(MULTITHREADED)
        for(unsigned long i=0; i<ULONG_MAX; i++){
            Strategy strategy = Strategy::generate(strategyGenConfig);
            Backtest backtest = doBackTest(strategy, dataset);
            if(backtest.metrics.metric() > 0){
                topN.insert(saveGeneratedStrategy(backtest, getNewName(), 0));
            }
            numEvaluated += 1;
            if (omp_get_thread_num() == 0 and omp_get_wtime() - start_time > 0.5){
                start_time = omp_get_wtime();
                showTopN(topN.getSorted(), "Evaluated: " + std::to_string(numEvaluated), true);
            }
        }
//        double time = omp_get_wtime() - start_time;
        std::cout << "\n" << time << std::endl;
    }

    std::string getNewName(){
        std::string name = get_random_name();
        while (jsonDB["strategies"].contains(name)){
            name = get_random_name();
        }
        return name;
    }

    GeneratedStrategy saveGeneratedStrategy(const Backtest &backtest, std::string name, int version) {
        GeneratedStrategy strategy = GeneratedStrategy::fromBacktest(name, version, backtest);
        if(!jsonDB["strategies"].contains(name)){
            jsonDB["strategies"][name] = json::object({});
            jsonDB["strategies"][name]["versions"] = json::array();
        }
        jsonDB["strategies"][name]["versions"].push_back(strategy.toJson());
        jsonDB["strategies"][name]["defaultVersion"] = jsonDB["strategies"][name]["versions"].size() - 1;
        JsonFileHandler::write(app->get_option("--database")->as<std::string>(), jsonDB);
        return strategy;
    }



    void showTopN( const std::vector<GeneratedStrategy>& vect, std::string header = "", bool rolling = false){

        fort::char_table table;
        table.set_cell_text_align(fort::text_align::center);
        table.set_border_style(FT_BOLD_STYLE);
        table.row(0).set_cell_bg_color(fort::color::black);
        table[0][0].set_cell_span(11);
        if(header.length() > 0){
            table <<  fort::header  << header << fort::endr;
        } else {
            table <<  fort::header  << "Generated Strategies" << fort::endr;
        }

        table << fort::header << "Name"  << "Version" << "CAGR/AvgDD" << "# Trades" << "Max DD (%)" << "Profit Factor" << "Win %" << "Return %" << "CAGR (%)" << "Dataset" << "Created At" << fort::endr << fort::separator;
        for(auto& b: vect){
            json bJson = b.toJson();
            json metrics = bJson["metrics"];
            json dataset = bJson["dataset"];
            table << b.name <<  b.version << metrics["CAGROverAvgDrawDown"].get<double>()
                  << metrics["numTrades"].get<int>() << metrics["maxDrawDown"].get<double>()*100
                  << metrics["profitFactor"].get<double>() << metrics["winRate"].get<double>()*100
                  << metrics["totalReturn"].get<double>() *100 << metrics["CAGR"].get<double>() *100
                  << dataset["baseAsset"].get<std::string>() + "/" + dataset["quoteAsset"].get<std::string>() + " (" + dataset["interval"].get<std::string>() + ")"
                  << bJson["createdAt"].get<std::string>().substr(0, 19) << fort::endr << fort::separator;
        }

        auto table_str = table.to_string();

        if(rolling){
            int numNewLines = std::count(table_str.begin(), table_str.end(), '\n');
            std::cout << table_str << "\033[100D" << "\033[" + std::to_string(numNewLines) + "A";
        } else{
            std::cout << table_str << std::endl;
        }
    }
};
#endif //CLI_GENERATE_H
