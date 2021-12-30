//
// Created by Abhishek Aggarwal on 21/12/2021.
//

#ifndef CLI_APP_H
#define CLI_APP_H

#include "../include/CLI11.hpp"
#include "configure.h"
#include "data.h"


class CryptoniteApp {
    std::vector<std::shared_ptr<CryptoniteCommand>> commands{};
public:
    CLI::App app{"Cryptonite Application is a series of commands that lets you generate, backtest and"
                 "optimise a trading strategy.", "cryptonite"};
    CryptoniteApp() {
        app.get_formatter()->column_width(80);
        app.remaining_for_passthrough(true);
        app.option_defaults()->multi_option_policy();
        app.set_help_all_flag("--help-all", "Expand all help");
        app.require_subcommand(1); // require one subcommand, either edit, generate, optimize
        std::string databasePath = "cryptonite.db";
        std::string dataStorePath = "datastore";
        app.add_option("--database, -d", databasePath, "Path to Cryptonite database file, created if non-existent.")
                ->capture_default_str()
                ->check(CLI::NonexistentPath | CLI::ExistingPath)
                ->envname("CRYPTONITE_DB");

        app.add_option("--datastore, -s", dataStorePath, "Path to data folder that contains datasets, created if non-existent.")
                ->capture_default_str()
                ->check(CLI::NonexistentPath | CLI::ExistingPath)
                ->envname("CRYPTONITE_DS");

    }

    void addCommand(std::shared_ptr<CryptoniteCommand> command){
        commands.push_back(command);
    }

    void setup(){
        for(auto command: commands){
            command->setup(app);
        }
    }

    void parse(){
        for(auto command: commands){
            command->parse();
        }
    }

    int run(int argc, char **argv){
        addCommand(std::shared_ptr<CryptoniteCommand>{new Configure});
        addCommand(std::shared_ptr<CryptoniteCommand>{new Data});

        setup();

        CLI11_PARSE(app, argc, argv);

        try {                                                                                                              \
            parse();
//            Database db("Hello.txt");
                                                                                            \
        } catch (const std::system_error& e) {
            std::cout << e.what() << " (" << e.code() << ")" << std::endl;
        }

        return 0;
    }
};


#endif
