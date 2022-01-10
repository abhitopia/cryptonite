//
// Created by Abhishek Aggarwal on 31/12/2021.
//

#ifndef CLI_GENERATE_H
#define CLI_GENERATE_H

#include <unordered_map>
#include <limits>
#include "../include/CLI11.hpp"
#include "../src/config.h"
#include "../src/dataset.h"
#include "../src/indicator.h"
#include "../src/backtest.h"
#include "command.h"
#include "../src/top_n_container.h"


struct GeneratedStrategy {
    std::string name{};
    json content{};
    std::string createdAt{};
    GeneratedStrategy(){};

    static GeneratedStrategy fromBacktest(std::string name, const Backtest& backtest){
        auto backtestJson = backtest.toJson();
        auto generatedStrategy = GeneratedStrategy{};
        generatedStrategy.name = name;
        generatedStrategy.content["metrics"] = backtestJson["metrics"];
        generatedStrategy.content["strategy"] = backtestJson["strategy"];
        generatedStrategy.content["dataset"] = backtestJson["dataset"];
        generatedStrategy.createdAt = date::format("%F %T", std::chrono::system_clock::now());
        return generatedStrategy;
    }

    json toJson() const {
        json j;
        j["content"] = content;
        j["createdAt"] = createdAt;
        return j;
    }

    double metric() const {
        return content["metrics"]["CAGROverAvgDrawDown"].get<double>() * content["metrics"]["numTrades"].get<double>();
    }

    bool operator>(const GeneratedStrategy& other) const {
        return this->metric() > other.metric();
    }
};


struct Generate: CryptoniteCommand {
    json jsonDB{};

    void parse() override {
        if(command->count() > 0){
            jsonDB = JsonFileHandler::read(app->get_option("--database")->as<std::string>(), true);
            if(!jsonDB.contains("generated"))
                jsonDB["generated"] = json::object({});

            std::string name = command->get_option("--config")->as<std::string>();
            int version =  command->get_option("--version")->count() > 0 ? command->get_option("--version")->as<int>() : -1;

            if(!hasConfig(name, version)){
                std::string versionUsed = version < 0 ? "default" : std::to_string(version);
                std::cout << "Configuration `" << name << "` (version: " << versionUsed << ") does not exist!" << std::endl;
                return;
            }

            auto config = getConfig(name, version);
            showConfig(name, version);

            generate(config);
        }
    }


    void setup(CLI::App& cli_app) override {
        app = &cli_app;
        command = app->add_subcommand("generate", "Generate a strategy");

        command->add_option("--config,-c,config", "Name of the configuration.")
                    ->check(CLI::TypeValidator<std::string>())
                    ->required();

        command->add_option("--version,-v,version", "Version of the configuration. Default version used if not specified")
                    ->check(CLI::PositiveNumber);
    }



private:
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
        Backtester backtester{strategyGenConfig, app->get_option("--datastore")->as<std::string>()};

        double start_time = omp_get_wtime();
        auto topN = TopNContainer<GeneratedStrategy>{10};
        unsigned long numEvaluated = 0;
        #pragma omp parallel for default(none) shared(numEvaluated, start_time, topN, backtester, strategyGenConfig)
        for(unsigned long i=0; i<ULONG_MAX; i++){
            Strategy strategy = Strategy::generate(strategyGenConfig);
            Backtest backtest = backtester.evaluate(strategy);
            if(backtest.metrics.metric() > 0){
                topN.insert(saveGeneratedStrategy(backtest));
            }
            numEvaluated += 1;
            if (omp_get_thread_num() == 0 and omp_get_wtime() - start_time > 0.5){
                start_time = omp_get_wtime();
                showTopN(numEvaluated, topN.getSorted());
            }
        }
//        double time = omp_get_wtime() - start_time;
        std::cout << "\n" << time << std::endl;
    }

    GeneratedStrategy saveGeneratedStrategy(const Backtest& backtest){
        std::string name = get_random_name();
        while (jsonDB["generate"].contains(name)){
            name = get_random_name();
        }

        GeneratedStrategy strategy = GeneratedStrategy::fromBacktest(name, backtest);
        jsonDB["generated"][name] = strategy.toJson();
        JsonFileHandler::write(app->get_option("--database")->as<std::string>(), jsonDB);
        return strategy;
    }

    void showTopN(int numEvaluated, const std::vector<GeneratedStrategy>& vect){

        fort::char_table table;
        table.set_cell_text_align(fort::text_align::center);
        table.set_border_style(FT_BOLD_STYLE);
        table.row(0).set_cell_bg_color(fort::color::black);
        table[0][0].set_cell_span(9);
        table <<  fort::header  << "Evaluated: " + std::to_string(numEvaluated) << fort::endr;
        table << fort::header << "Name"  << "CAGR/AvgDD" << "# Trades" << "Max DD (%)" << "Profit Factor" << "Win %" << "Return %" << "CAGR (%)" << "Created At" << fort::endr << fort::separator;
        for(auto& b: vect){
            json metrics = b.toJson()["content"]["metrics"];
            table << b.name <<  metrics["CAGROverAvgDrawDown"].get<double>()
                  << metrics["numTrades"].get<int>() << metrics["maxDrawDown"].get<double>()*100
                  << metrics["profitFactor"].get<double>() << metrics["winRate"].get<double>()*100
                  << metrics["totalReturn"].get<double>() *100 << metrics["CAGR"].get<double>() *100
                  << b.createdAt << fort::endr << fort::separator;
        }

        auto table_str = table.to_string();
        int numNewLines = std::count(table_str.begin(), table_str.end(), '\n');
        std::cout << table_str << "\033[100D" << "\033[" + std::to_string(numNewLines) + "A";


    }
};
#endif //CLI_STRATEGY_H
