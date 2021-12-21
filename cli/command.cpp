//
// Created by Abhishek Aggarwal on 21/12/2021.
//

#include "../include/CLI11.hpp"


struct CryptoniteCommand {
    CLI::App *app;
    CLI::App *command;

    virtual void parse() = 0;
    virtual void setup(CLI::App &app) = 0;
};