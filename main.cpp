#include "cli/app.h"


int main(int argc, char **argv) {
    cryptonite::seed(); // Ensure that this called only once!
//    test_genetic_algorithm();
    CryptoniteApp app{};
    return app.run(argc,argv);
}


//#include <atomic>
//#include <chrono>
//#include <random>
//#include "include/tabulate.hpp"
//#include <thread>
//
//
//using namespace tabulate;
//using Row_t = Table::Row_t;
//std::atomic_bool keep_running(true);
//
//void waitingForWorkEnterKey() {
//    while (keep_running) {
//        if (std::cin.get() == 10) {
//            keep_running = false;
//        }
//    }
//    return;
//}
//
//int main() {
//    std::thread tUserInput(waitingForWorkEnterKey);
//    while (keep_running) {
//        Table process_table;
//        std::random_device rd;
//        std::mt19937 gen(rd());
//        std::uniform_real_distribution<> dis(0, 1);
//        process_table.add_row(Row_t{"Ligita", "%CPU", "%MEM", "User", "NI"});
//        process_table.add_row(
//                Row_t{"4297", std::to_string((int)round(dis(gen) * 100)),
//                      std::to_string((int)round(dis(gen) * 100)), "ubuntu", "20"});
//        process_table.add_row(
//                Row_t{"12671", std::to_string((int)round(dis(gen) * 100)),
//                      std::to_string((int)round(dis(gen) * 100)), "root", "0"});
//        process_table.add_row({"810", std::to_string((int)round(dis(gen) * 100)),
//                               std::to_string((int)round(dis(gen) * 100)), "root",
//                               "-20"});
//
//        process_table.column(2).format().font_align(FontAlign::center);
//        process_table.column(3).format().font_align(FontAlign::right);
//        process_table.column(4).format().font_align(FontAlign::right);
//
//        for (size_t i = 0; i < 5; ++i) {
//            process_table[0][i]
//                    .format()
//                    .font_color(Color::yellow)
//                    .font_align(FontAlign::center)
//                    .font_style({FontStyle::bold});
//        }
//
//        std::cout << process_table << std::endl;
//        std::cout << "Press ENTER to exit..." << std::endl;
//        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//        std::cout << "\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F";
//    }
//    std::cout << "\033[B\033[B\033[B\033[B\033[B\033[B\033[B\033[B\033[B\033[B";
//    tUserInput.join();
//
//    return 0;
//}


// {"brokerConfig":{"commission":0.002,"slippage":0.005},"depositConfig":{"maxBaseBorrow":-1.0,"quoteDeposit":1000.0},"entryCriteria":[{"Indicator":{"name":"StandardDeviation"},"params":{"apply_to":1.0,"level":0.0,"period":20.0},"trigger":{"comparand":"value","comparator":"","name":"changes_direction_upward"}}],"exitCriteria":[{"Indicator":{"name":"MoneyFlowIndex"},"params":{"level":20.0,"period":14.0},"trigger":{"comparand":"value","comparator":"","name":"changes_direction_upward"}},{"Indicator":{"name":"BollingerBands"},"params":{"apply_to":2.0,"deviation":2.0,"period":20.0},"trigger":{"comparand":"bar","comparator":"lower","name":"crosses_upward"}}],"positionCloseConfig":{"stopLoss":0.04008713421946008,"takeProfit":0.10488679296883245,"trailingSl":true},"positionOpenConfig":{"bidirectional":true,"isAbsolute":false,"quoteSize":1.0}}
// {"brokerConfig":{"commission":0.002,"slippage":0.005},"depositConfig":{"maxBaseBorrow":-1.0,"quoteDeposit":1000.0},"entryCriteria":[{"Indicator":{"name":"OnBalanceVolume"},"trigger":{"comparand":"value","comparator":"","name":"changes_direction_upward"}},{"Indicator":{"name":"ADX"},"params":{"level":3.1020240959857044,"period":12.0},"trigger":{"comparand":"value","comparator":"","name":"rises"}}],"exitCriteria":[{"Indicator":{"name":"Momentum"},"params":{"apply_to":2.0,"level":-7348.6346200962325,"period":14.0},"trigger":{"comparand":"value","comparator":"level","name":"crosses_upward"}}],"positionCloseConfig":{"stopLoss":null,"takeProfit":null,"trailingSl":false},"positionOpenConfig":{"bidirectional":true,"isAbsolute":false,"quoteSize":1.0}}
// {"brokerConfig":{"commission":0.002,"slippage":0.005},"depositConfig":{"maxBaseBorrow":-1.0,"quoteDeposit":1000.0},"entryCriteria":[{"Indicator":{"name":"Momentum"},"params":{"apply_to":2.0,"level":-6269.3065741833525,"period":12.0},"trigger":{"comparand":"value","comparator":"level","name":"higher_than"}}],"exitCriteria":[{"Indicator":{"name":"ForceIndex"},"params":{"ma_method":0.0,"period":18.0},"trigger":{"comparand":"value","comparator":"","name":"rises"}}],"positionCloseConfig":{"stopLoss":null,"takeProfit":0.05601015630877239,"trailingSl":false},"positionOpenConfig":{"bidirectional":true,"isAbsolute":false,"quoteSize":1.0}}