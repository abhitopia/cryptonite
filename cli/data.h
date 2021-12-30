#ifndef CLI_DATA_H
#define CLI_DATA_H

#include "command.h"
#include "../include/date.h"
#include "../include/CLI11.hpp"
#include "../src/datastore.h"
#include "../include/fort/fort.hpp"


struct Data: CryptoniteCommand {
    void setup(CLI::App& cli_app) override {
        app = &cli_app;
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

        command = app->add_subcommand("data", "To get information for a dataset of a symbol/ticker.");
        command->add_option("--base-asset,-b,base-asset", "Symbol representing Base Asset, Eg. `BTC`")
               ->required(true);
        command->add_option("--quote-asset,-q,quote-asset", "Symbol representing Quote Asset, Eg. `GBP`")
                ->required(true);
        command->add_option("--interval,-i,interval", "OHLC Interval")
                ->required(true)
                ->transform(intervalValidator);
    }

    void parse() override {
        if(command->count() > 0){
            auto info = DataSetInfo(command->get_option("--base-asset")->as<std::string>(),
                                    command->get_option("--quote-asset")->as<std::string>(),
                                    stringToInterval(command->get_option("--interval")->as<std::string>()));

            if (!info.check_valid()){
                std::cout << "Symbol: " << info.symbol() << " with Base asset: " << info.baseAsset << " and Quote Asset: "
                << info.quoteAsset << " not found!" << std::endl;
                return;
            }

            auto ds = DataStore(info, app->get_option("--datastore")->as<std::string>());
            auto dataset = ds.getDataset();
            fort::char_table table;
            table.set_cell_text_align(fort::text_align::center);
            table.set_border_style(FT_BOLD_STYLE);
            table.row(0).set_cell_bg_color(fort::color::black);
//            table.row(1).set_cell_bg_color(fort::color::black);
            table[0][0].set_cell_span(2);

            std::uint32_t time_date_stamp = dataset->timestamp->front();
            date::sys_seconds tp{std::chrono::seconds{time_date_stamp}};
            std::string startDate = date::format("%Y-%m-%d %I:%M:%S %p", tp);
            std::cout << std::endl;
            table <<  fort::header  << "Symbol: " + info.symbol() << fort::endr;
            table << "Number of bars" << dataset->numBars() << fort::endr << fort::separator;
            double pctVolume = (100.0 * dataset->barsWithVolume())/ dataset->numBars();
            table << "Bars with volume (%)" << std::to_string(dataset->barsWithVolume()) + "  (" + std::to_string(pctVolume) + "% )"  << fort::endr << fort::separator;
            table << "Interval" << info.intervalInString() << fort::endr << fort::separator;
            table << "Start TimeStamp" << startDate << fort::endr << fort::separator;
            table << "Exchange Info" << std::setw(2) << info.exchangeInfo() << fort::endr;
            std::cout << table.to_string() << std::endl;

        }
    }

};

#endif
