//
// Created by Abhishek Aggarwal on 21/12/2021.
//
#include "../include/CLI11.hpp"
#include "../src/config.h"
#include "command.cpp"


using namespace std;


struct Configure: CryptoniteCommand {
    StrategyGenConfig config{};

    void parse() override {
        if(command->count() > 0){
            cout << "Command called " << endl;
        }
    }

    void setup(CLI::App& cli_app) override {
        app = &cli_app;
        command = app->add_subcommand("config", "For management of configurations.");
        command->require_subcommand(1);
        add_subcommand("add", "Add a new configuration.", false, true);
        add_subcommand("update", "Update an existing configuration.", true, true);
        add_subcommand("version", "Set/Get default version of the configuration.", true)
            ->get_option("--version")->description("Specify the default version of the configuration.");
        add_subcommand("show", "Shows the contents of the configuration.", true)
            ->get_option("--version")->description("If not specified, default version is used.");
        auto removeSubcommand = add_subcommand("remove", "Deletes a/all version of a configuration.", true);

        removeSubcommand->get_option("--version")->description("Version to be deleted. Default version cannot be deleted.");
        removeSubcommand->add_flag("--all,-a",  "Delete all the versions.")->excludes("--version");
        removeSubcommand->get_option("--version")->excludes("-a");
    }

private:
    CLI::App *add_subcommand(string name, string description, bool addVersion = true, bool withEditingOptions = false) {
        CLI::App *subcommand = command->add_subcommand(name, description);
        subcommand->add_option("name", "Name of the configuration.")
                ->required();
        if (addVersion) {
            subcommand->add_option("--version,-v,version", "Version of the configuration.")
                    ->check(CLI::PositiveNumber)
                    ->check(CLI::TypeValidator<int>());
        }

        if(withEditingOptions){
            addEditingOptions(subcommand);
        }

        return subcommand;
    }

    void addEditingOptions(CLI::App* subcommand){

        // Rules
        auto rulesConfig = subcommand->add_option_group("Rules Configuration");
        rulesConfig->add_option("--max-entry-rules,--mnr", config.rulesGenConfig.numMaxEntryRules,
                                "Maximum number of entry rules used in generated strategies")
                ->check(CLI::Range(1, 6))
                ->check(CLI::TypeValidator<int>())
                ->capture_default_str();
        rulesConfig->add_option("--max-exit-rules,--mxr", config.rulesGenConfig.numMaxExitRules,
                                "Maximum number of exit rules used in generated strategies")
                ->check(CLI::Range(1, 6))
                ->check(CLI::TypeValidator<int>())
                ->capture_default_str();
        rulesConfig->add_option("--exploration-prob,--ep", config.rulesGenConfig.explorationProb,
                                "This number controls probability with which the generated rule parameters deviate from their default values.")
                ->check(CLI::Range(0.01, 1.0));


        // Trade Size
        auto tradeSize = subcommand->add_option_group("Trade Size Configuration");
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
        auto takeProfit = subcommand->add_option_group("Take Profit Configuration", "");
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
        auto stopLoss = subcommand->add_option_group("Stop Loss Configuration", "");
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
        auto brokerConfig = subcommand->add_option_group("Broker Configuration");
        brokerConfig->add_option("--commission,-c", config.brokerConfig.commission, "Commission per closed trade")->check(CLI::Range(0.0, 1.0))
                ->capture_default_str();
        brokerConfig->add_option("--slippage, -s", config.brokerConfig.slippage, "Slippage to be applied when opening/closing a trade")->check(CLI::Range(0.0, 1.0))
                ->capture_default_str();

        // Deposit Config
        auto depositConfig = subcommand->add_option_group("Deposit Configuration");
        depositConfig->add_option("--quote-deposit,-d", config.depositConfig.quoteDeposit, "Opening balance in quote units.")
                ->check(CLI::PositiveNumber)
                ->capture_default_str();
        depositConfig->add_option("--max-base-borrow,--mbb", config.depositConfig.maxBaseBorrow,
                                  "Specifies amount of base units available to borrow for short-selling.\nNegative Values mean infinite borrowing allowance (Bidirectional Trades only)")
                ->capture_default_str()
                ->excludes(neverBi);
    }
};

