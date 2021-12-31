//
// Created by Abhishek Aggarwal on 21/12/2021.
//

#ifndef CLI_CONFIGURE_H
#define CLI_CONFIGURE_H

#include "../include/CLI11.hpp"
#include "../src/config.h"
#include "../src/json_file_handler.h"
#include <cmath>
#include <map>
#include "../include/date.h"
#include "../include/fort/fort.hpp"

#include "command.h"

std::string getTimestamp(){
    return date::format("%F %T", std::chrono::system_clock::now());
}


json cmdToJson(CLI::App* app, bool default_also){
    json j = json::object({});

    for(const CLI::Option *opt : app->get_options({})) {
        // Only process option with a long-name and configurable
        if(!opt->get_lnames().empty()) {
            std::string name = opt->get_lnames()[0];

            // Non-flags
            if(opt->get_type_size() != 0) {

                // If the option was found on command line
                if(opt->count() == 1)
                    j[name] = opt->results().at(0);
                else if(opt->count() > 1)
                    j[name] = opt->results();

                    // If the option has a default and is requested by optional argument
                else if(default_also && !opt->get_default_str().empty()){
                    j[name] = opt->get_default_str();
                }
                // Flag, one passed
            } else if(opt->count() == 1) {
                j[name] = true;

                // Flag, multiple passed
            } else if(opt->count() > 1) {
                j[name] = opt->count();

                // Flag, not present
            } else if(opt->count() == 0 && default_also) {
                j[name] = false;
            }
        }
    }

    return j;
}

class EditingOptionFormatter : public CLI::Formatter {
    bool displayDefault{true};
public:
    EditingOptionFormatter(bool displayDefault) : Formatter() {
        this->displayDefault = displayDefault;
    };

    std::string make_option_opts(const CLI::Option *opt) const {
        std::stringstream out;

        if (!opt->get_option_text().empty()) {
            out << " " << opt->get_option_text();
        } else {
            if (opt->get_type_size() != 0) {
                if (!opt->get_type_name().empty())
                    out << " " << get_label(opt->get_type_name());
                if (!opt->get_default_str().empty() and displayDefault)
                    out << "=" << opt->get_default_str();
                if (opt->get_expected_max() == CLI::detail::expected_max_vector_size)
                    out << " ...";
                else if (opt->get_expected_min() > 1)
                    out << " x " << opt->get_expected();

                if (opt->get_required())
                    out << " " << get_label("REQUIRED");
            }
            if (!opt->get_envname().empty())
                out << " (" << get_label("Env") << ":" << opt->get_envname() << ")";
            if (!opt->get_needs().empty()) {
                out << " " << get_label("Needs") << ":";
                for (const CLI::Option *op: opt->get_needs())
                    out << " " << op->get_name();
            }
            if (!opt->get_excludes().empty()) {
                out << " " << get_label("Excludes") << ":";
                for (const CLI::Option *op: opt->get_excludes())
                    out << " " << op->get_name();
            }
        }
        return out.str();
    }
};

struct Configure: CryptoniteCommand {
    json jsonDB{};

    void parse() override {
        if(command->count() > 0){
            jsonDB = JsonFileHandler::read(app->get_option("--database")->as<std::string>(), true);
            if(!jsonDB.contains("configs"))
                jsonDB["configs"] = json::object({});

            CLI::App* subcommand{nullptr};
            for(auto& subcmd : command->get_subcommands()){
                if(subcmd->parsed()){
                    subcommand = subcmd;
                    break;
                }
            }
            
            std::string name = subcommand->get_option("name")->as<std::string>();
            int version{-1}; // negative means default version

            if(subcommand->get_name() != "add"){
                version = subcommand->get_option("version")->count() > 0 ? subcommand->get_option("-v")->as<int>() : -1;
            }

            bool removeAll{false};
            json config = json::object({});

            switch (switchHash(subcommand->get_name().c_str())) {
                case switchHash("add"):
                    if(hasConfig(name, version)){
                        std::cout << "Configuration `" << name << "` already exists!" << std::endl;
                        return;
                    }
                    config = cmdToJson(subcommand, true);
                    if(!validateConfig(config)) return;
                    setConfig(name, -1, config);
                    list(name, getDefaultVersion(name), true);
                    break;
                case switchHash("update"):
                    if(!hasConfig(name, version)){
                        std::cout << "Configuration `" << name << "` does not exist!" << std::endl;
                        return;
                    }
                    version = version <= 0 ? getDefaultVersion(name) : version;
                    config = getConfig(name, version);
                    if(!updateConfig(config, cmdToJson(subcommand, false))){
                        std::cout << "No changes detected. Skipping update!" << std::endl;
                        return;
                    }
                    if(!validateConfig(config)) return;
                    setConfig(name, version, config);
                    list(name, getDefaultVersion(name), true);
                    break;
                case switchHash("version"):
                    if(!hasConfig(name, version)){
                        std::string versionUsed = version < 0 ? "default" : std::to_string(version);
                        std::cout << "Configuration `" << name << "` (version: " << versionUsed << ") does not exist!" << std::endl;
                        return;
                    }
                    if(version <= 0){
                        // get default version
                        std::cout << "Default version is: " << getDefaultVersion(name) << std::endl;
                        list(name, getDefaultVersion(name), true);
                    } else {
                        setDefaultVersion(name, version);
                        list(name, version, true);

                    }
                    break;
                case switchHash("remove"):
                    removeAll = subcommand->get_option("--all")->as<bool>();
                    if(!hasConfig(name, version)){
                        std::string versionUsed = removeAll ? "any" : (version < 0 ? "default" : std::to_string(version));
                        std::cout << "Configuration `" << name << "` (version: " << versionUsed << ") does not exist!" << std::endl;
                        return;
                    }
                    remove(name, version, removeAll);
                    break;
                case switchHash("list"):
                    if(subcommand->get_option("name")->count() and !hasConfig(name, version)){
                        std::string versionUsed = version < 0 ? "default" : std::to_string(version);
                        std::cout << "Configuration `" << name << "` (version: " << versionUsed << ") does not exist!" << std::endl;
                        return;
                    }
                    list(name, version, subcommand->get_option("--version")->count() > 0);
                    break;
            }

            JsonFileHandler::write(app->get_option("--database")->as<std::string>(), jsonDB);
        }
    }

    void setup(CLI::App& cli_app) override {
        app = &cli_app;
//        std::shared_ptr<Database> dbase(new Database(app->get_option("--database")->as<std::string>()));
//        db.swap(dbase);
        command = app->add_subcommand("config", "For management of configurations.");
        command->require_subcommand(1);
        add_subcommand("add", "Add a new configuration.", false, true, true);
        add_subcommand("update", "Update an existing configuration. Default values are those of base configuration.", true, true, false);
        add_subcommand("version", "Set/Get default version of the configuration.", true)
            ->get_option("--version")
            ->description("Specify the default version of the configuration.");
//        add_subcommand("show", "Shows the contents of the configuration.", true)
//            ->get_option("--version")->description("If not specified, default version is used.");
        auto removeSubcommand = add_subcommand("remove", "Deletes a/all version of a configuration.", true);
        removeSubcommand->require_option(2);
        removeSubcommand->get_option("--version")->description("Version to be deleted. Default version cannot be deleted.");
        removeSubcommand->add_flag("--all,-a",  "Delete all the versions.")->excludes("--version");
        removeSubcommand->get_option("--version")->excludes("-a");

        auto listSubcommand = add_subcommand("list", "List all available configurations.", true, false);
        listSubcommand->require_option(0);
        listSubcommand->get_option("name")->description("Only list versions of this configuration.")->required(false);
        listSubcommand->get_option("--version")->needs("name");

    }

private:
    CLI::App *add_subcommand(std::string name, std::string description, bool addVersion = true, bool withEditingOptions = false,
                             bool showDefaults=false) {
        CLI::App *subcommand = command->add_subcommand(name, description);
        subcommand->add_option("name", "Name of the configuration.")
                ->required();
        if (addVersion) {
            subcommand->add_option("--version,-v,version", "Version of the configuration.")
                    ->check(CLI::PositiveNumber)
                    ->check(CLI::TypeValidator<int>());
        }

        if(withEditingOptions){
            addEditingOptions(subcommand, showDefaults);
        }

        return subcommand;
    }

    void addEditingOptions(CLI::App* subcommand, bool showDefault){
        // Rules
        subcommand->formatter(std::make_shared<EditingOptionFormatter>(showDefault));
        subcommand->get_formatter()->column_width(app->get_formatter()->get_column_width());
        StrategyGenConfig config{};

        subcommand->add_option("--min-num-trades,--mnt", "Minimum number of trades in the backtest.")
                  ->check(CLI::PositiveNumber & CLI::TypeValidator<int>())
                  ->default_val(config.acceptanceConfig.minNumTrades);


        auto intervalValidator = CLI::IsMember({intervalToString(Interval::MINUTE1),
                                                intervalToString(Interval::MINUTE3),
                                                intervalToString(Interval::MINUTE5),
                                                intervalToString(Interval::MINUTE15),
                                                intervalToString(Interval::MINUTE30),
                                                intervalToString(Interval::HOUR1),
                                                intervalToString(Interval::HOUR2),
                                                intervalToString(Interval::HOUR4),
                                                intervalToString(Interval::HOUR6),
                                                intervalToString(Interval::HOUR8),
                                                intervalToString(Interval::HOUR12),
                                                intervalToString(Interval::DAY1),
                                                intervalToString(Interval::DAY3),
                                                intervalToString(Interval::WEEK1),
                                                intervalToString(Interval::MONTH1)
                                               });

        subcommand->add_option("--base-asset,-b", "Base Asset eg. BTC")
                  ->check(CLI::TypeValidator<std::string>())
                  ->default_val(config.dataSetConfig.baseAsset);

        subcommand->add_option("--quote-asset,-q", "Quote Asset eg. GBP")
                ->check(CLI::TypeValidator<std::string>())
                ->default_val(config.dataSetConfig.quoteAsset);

        subcommand->add_option("--interval,-i", "Dataset interval between bars")
                ->transform(intervalValidator)
                ->default_val(config.dataSetConfig.intervalInString());

        subcommand->add_option("--max-entry-rules,--mnr",
                                "Maximum number of entry rules used in generated strategies")
                ->check(CLI::Range(1, 6))
                ->default_val(config.rulesGenConfig.numMaxEntryRules)
                ->check(CLI::TypeValidator<int>());
        subcommand->add_option("--max-exit-rules,--mxr",
                                "Maximum number of exit rules used in generated strategies")
                ->check(CLI::Range(1, 6))
                ->default_val(config.rulesGenConfig.numMaxExitRules)
                ->check(CLI::TypeValidator<int>());
        subcommand->add_option("--exploration-prob,--ep",
                                "This number controls probability with which the generated rule parameters deviate from their default values.")
                ->check(CLI::Range(0.01, 1.0))
                ->default_val(config.rulesGenConfig.explorationProb);


        auto policyValidator = CLI::IsMember({policyToString(Policy::ALWAYS), policyToString(Policy::NEVER), policyToString(Policy::SOMETIMES)}, CLI::ignore_case);
        auto slTypeValidator = CLI::IsMember({slTypeToString(SLType::TRAILING), slTypeToString(SLType::FIXED), slTypeToString(SLType::EITHER)}, CLI::ignore_case);


        // Trade Size
        std::string defaultBidirectionalPolicy = policyToString(config.tradeSizeGenConfig.bidirectionalTradePolicy);
        std::string defaultFixedSizePolicy = policyToString(config.tradeSizeGenConfig.fixedTradeSizePolicy);
        std::string defaultTPPolicy = policyToString(config.takeProfitGenConfig.policy);
        std::string defaultSLPolicy = policyToString(config.stopLossGenConfig.policy);

        subcommand->add_option("--bidirectional-policy,--bip", "Whether to generate bidirectional (long and short) strategies.")
                            ->transform(policyValidator)
                            ->default_val(defaultBidirectionalPolicy);

        subcommand->add_option("--fixed-size-policy,--fip", "Whether generated strategies can use fixed trade size.")
                ->transform(policyValidator)
                ->default_val(defaultFixedSizePolicy);


        // Take Profit
        subcommand ->add_option("--tp-policy,--tpp", "Whether to use take-profit when opening Trade.")
                ->transform(policyValidator)
                ->default_val(defaultTPPolicy);

        subcommand->add_option("--tp-min,--tpm", "Minimum take-profit generated.")
                ->ignore_case(false)
                ->check(CLI::Range(0.01, 0.1))
                ->default_val(config.takeProfitGenConfig.tpMin);
        subcommand->add_option("--tp-max,--tpM", "Maximum take-profit generated.")
                ->ignore_case(false)
                ->check(CLI::Range(0.1, 0.2))
                ->default_val(config.takeProfitGenConfig.tpMax);

        //Stop Loss
        subcommand->add_option("--sl-policy,--slp", "Whether to use stop-loss when opening Trade.")
                    ->transform(policyValidator)
                    ->default_val(defaultSLPolicy);

        subcommand->add_option("--sl-min,--slm", "Minimum stop-loss generated.")
                ->ignore_case(false)
                ->check(CLI::Range(0.01, 0.1))
                ->default_val(config.stopLossGenConfig.slMin);
        subcommand->add_option("--sl-max,--slM", "Maximum stop-loss generated.")
                ->ignore_case(false)
                ->check(CLI::Range(0.1, 0.2))
                ->default_val(config.stopLossGenConfig.slMax);


        std::string defaultSLType = slTypeToString(config.stopLossGenConfig.type);
        subcommand->add_option("--sl-type,--slt", "Type of stop-loss generated.")
                ->transform(slTypeValidator)
                ->default_val(defaultSLType);


        // Broker Config
        subcommand->add_option("--commission,-c",  "Commission per closed trade")
                    ->check(CLI::Range(0.0, 1.0))
                    ->default_val(config.brokerConfig.commission);
        subcommand->add_option("--slippage, -s",  "Slippage to be applied when opening/closing a trade")
                    ->check(CLI::Range(0.0, 1.0))
                    ->default_val(config.brokerConfig.slippage);

        // Deposit Config
        subcommand->add_option("--quote-deposit,-d",  "Opening balance in quote units.")
                ->check(CLI::PositiveNumber)
                ->default_val(config.depositConfig.quoteDeposit);
        subcommand->add_option("--max-base-borrow,--mbb",
                                  "Specifies amount of base units available to borrow for short-selling.\nNegative Values mean infinite borrowing allowance (Bidirectional Trades only)")
                                  ->check(CLI::Number)
                                  ->default_val(config.depositConfig.maxBaseBorrow);
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

    void setConfig(std::string name, int parentVersion, json content){
        if(not jsonDB.contains("configs")){
            jsonDB["configs"] = json::object({});
        }

        json configContainer = json::object({});
        configContainer["content"] = content;
        configContainer["createdAt"] = getTimestamp();

        if(jsonDB["configs"].empty() || !jsonDB["configs"].contains(name)){
            json metaConfig;
            metaConfig["defaultVersion"] = 1;
            metaConfig["versions"] = json::object({});
            configContainer["parentVersion"] = -1;
            metaConfig["versions"]["1"] = configContainer;
            jsonDB["configs"][name] = metaConfig;
            return;
        }

        json& metaConfig = jsonDB["configs"][name];
        int nextVersion = -1;
        for (auto&[configVersion, config]: metaConfig["versions"].items()) {
            nextVersion = std::max(nextVersion, atoi(configVersion.c_str()) + 1);
        }
        configContainer["parentVersion"] = parentVersion;
        metaConfig["versions"][std::to_string(nextVersion)] = configContainer;
        metaConfig["defaultVersion"] = nextVersion;
        return;
    }

    int getDefaultVersion(std::string name){
        return jsonDB["configs"][name]["defaultVersion"].get<int>();
    }

    void setDefaultVersion(std::string name, int version){
        jsonDB["configs"][name]["defaultVersion"] = version;
    }

    int getLastVersion(std::string name){
        json metaConfig = jsonDB["configs"][name];
        int lastVersion = -1;
        for(auto& [version, _]: metaConfig["versions"].items()){
            lastVersion = std::max(lastVersion,  atoi(version.c_str()));
        }
        return lastVersion;
    }

    void remove(std::string name, int version, bool all=false){
        if(all){
            jsonDB["configs"].erase(name);
            return;
        }

        version = version <= 0 ? getDefaultVersion(name) : version;
        int parentVersion = jsonDB["configs"][name]["parentVersion"].get<int>();
        jsonDB["configs"][name].erase(std::to_string(version));

        if(parentVersion < 0){
            jsonDB["configs"].erase(name);
            return;
        }
        setDefaultVersion(name, parentVersion);
    }

    bool updateConfig(json& oldConfig, json newConfig){
        if(newConfig.empty()){
            return false;
        }
        bool valueUpdated{false};
        for(auto& [key, val] : newConfig.items()){
            if(oldConfig[key] != val){
                valueUpdated = true;
                oldConfig[key] = val;
            }
        }
        return valueUpdated;
    }

    void list(std::string name, int version, bool useVersion=false){
        fort::char_table table;
        table.set_cell_text_align(fort::text_align::center);
        table.set_border_style(FT_BOLD_STYLE);
        table.row(0).set_cell_bg_color(fort::color::black);
        table.row(1).set_cell_bg_color(fort::color::black);
        if(name.length() == 0){
            table[0][0].set_cell_span(4);
            table <<  fort::header  << "Configuration" << fort::endr;
            table << fort::header << "Configuration" << "Default Version" << "Number of Versions" << "Last Updated"  << fort::endr;
            for(auto& [name, metaConfig]: jsonDB["configs"].items()){
                std::string lastUpdated = metaConfig["versions"][std::to_string(getLastVersion(name))]["createdAt"].get<std::string>();
                table << name << metaConfig["defaultVersion"] << metaConfig["versions"].size() << lastUpdated <<fort::endr << fort::separator;
            }
        } else {
            int defaultVersion = getDefaultVersion(name);
            version = version <= 0 ? defaultVersion : version;
            table[0][0].set_cell_span(4);
            table <<  fort::header  << "Configuration: " + name << fort::endr;
            table << fort::header << "Version" << "Parent Version" << "Created On" << "Content" << fort::endr;
            json& metaConfig = jsonDB["configs"][name];
            for(auto& [configVersion, config]: metaConfig["versions"].items()){
                if(!useVersion || version == atoi(configVersion.c_str())){
                    std::string versionStr = configVersion;
                    if(atoi(configVersion.c_str()) == defaultVersion){
                        versionStr += " (default)";
                    }
                    table << versionStr << config["parentVersion"] << config["createdAt"].get<std::string>() << std::setw(2) << config["content"] << fort::endr << fort::separator;
                }

            }
        }

        std::cout << table.to_string() << std::endl;
    }


    bool validateConfig(json& config){
//        std::cout << std::setw(2) << config << std::endl;
        auto info = DataSetConfig(config["base-asset"].get<std::string>(),
                                  config["quote-asset"].get<std::string>(),
                                  stringToInterval(config["interval"].get<std::string>()));

        if(!info.check_valid()){
            std::cout << "Symbol: " + info.symbol() + " not available!" << std::endl;
            return false;
        }
        return true;
    }
};

#endif //CRYPTONITE_DATABASE_H
