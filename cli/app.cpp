//
// Created by Abhishek Aggarwal on 21/12/2021.
//

#include "../include/CLI11.hpp"
#include "configure.cpp"

using namespace std;

class CryptoniteApp {
    vector<shared_ptr<CryptoniteCommand>> commands{};
public:
    CLI::App app{"Cryptonite Application is a series of commands that lets you generate, backtest and"
                 "optimise a trading strategy.", "cryptonite"};
    CryptoniteApp() {
        app.get_formatter()->column_width(80);
        app.remaining_for_passthrough(true);
        app.option_defaults()->multi_option_policy();
        app.set_help_all_flag("--help-all", "Expand all help");
        app.require_subcommand(1); // require one subcommand, either edit, generate, optimize
        string databasePath = "cryptonite.db";
        app.add_option("--database, -d", databasePath, "Path to Cryptonite database file, created if non-existent.")
                ->capture_default_str()
                ->check(CLI::NonexistentPath | CLI::ExistingPath)
                ->envname("CRYPTONITE_DB");
    }

    void addCommand(shared_ptr<CryptoniteCommand> command){
        commands.push_back(command);
    }

    void setup(){
        for(auto command: commands){
            command->setup(app);
        }
    }

    int run(int argc, char **argv){
        addCommand(shared_ptr<CryptoniteCommand>{new Configure});
        setup();
        CLI11_PARSE(app, argc, argv);
        return 0;
    }
};
