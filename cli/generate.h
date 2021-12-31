//
// Created by Abhishek Aggarwal on 31/12/2021.
//

#ifndef CLI_GENERATE_H
#define CLI_GENERATE_H

#include "../include/CLI11.hpp"
#include "../src/config.h"
#include "../src/dataset.h"
#include "../src/indicator.h"
#include "../src/backtest.h"
#include "command.h"


struct Generate: CryptoniteCommand {
    json jsonDB{};

    void parse() override {
        if(command->count() > 0){
            jsonDB = JsonFileHandler::read(app->get_option("--database")->as<std::string>(), true);
            if(!jsonDB.contains("configs"))
                jsonDB["configs"] = json::object({});

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
        int numIterations = 1000;
        progressbar bar(numIterations);
        #pragma omp parallel for default(none) shared(backtester, strategyGenConfig, numIterations, bar)
        for(int i=0; i<numIterations;i++){
            Strategy strategy = Strategy::generate(strategyGenConfig);
            backtester.evaluate(strategy);
            #pragma omp critical
            {
                bar.update();
            };
        }
        double time = omp_get_wtime() - start_time;
        std::cout << "\n" << time << std::endl;
    }

};
#endif //CLI_STRATEGY_H
