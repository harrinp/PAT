#include "Headers/Decision.hpp"

Runner::Runner(std::string table, std::string longerTable, std::string orderItemName, double takeProfit, double stopLoss) :
    Decider(true, 0),
    orderItemName(orderItemName),
    exec() {
    this->stopLoss    = stopLoss;
    this->takeProfit  = takeProfit;
    this->table       = table;
    this->longerTable = longerTable;
}

void Runner::run() {
    Pos    pos        = NOTHING;
    double bal        = 0;
    std::cout << "HERE" << '\n';
    while (true) {
        std::cout << "Current time: " << std::time(0) << '\n';
        bal = exec.getBalance();
        FullBar lastBar = FullBar(table, true);

        Price p = lastBar.convertToPrice();

        std::cout << "Last data   : " << lastBar.date << '\n';

        if (std::time(0) - lastBar.date > 60){
            std::cout << "Error: last bar of data was too long ago, are you running the analyzer?" << '\n';
            return;
        }

        exec.trades = Trade::translateTrades(exec.getTradesJson());

        if(exec.trades.size() == 0){
            std::cout << "No trades" << '\n';
        }

        for (int i = 0; i < (int)exec.trades.size(); i++) {
            if (exec.trades[i].profit < -1 *stopLoss * exec.trades[i].units * p.ask || exec.trades[i].profit > takeProfit * exec.trades[i].units * p.ask) {
                if (exec.trades[i].type == SHORT) {
                    std::cerr << "ending short" << '\n';
                    exec.buy(exec.trades[i].units, orderItemName);
                }else if (exec.trades[i].type == LONG) {
                    std::cout << "ending long" << '\n';
                    exec.sell(exec.trades[i].units, orderItemName);
                }
            }
        }
        FullBar lastBarHourly = FullBar(longerTable, true);
        if (lastBarHourly.date > lastHourlyDate) {

            lastHourlyDate = lastBarHourly.date;
            pos        = decide(&lastBarHourly);

            switch (pos) {
            case LONGING:
                exec.sell(bal * .01 * 50, orderItemName);
                std::cout << "Selling " << bal * .01 * 50 << '\n';
                break;

            case SHORTING:
                exec.buy(bal * .01 * 50, orderItemName);
                std::cout << "Buying " << bal * .01 * 50 << '\n';
                break;

            case NOTHING:
                std::cout << "Holding " << '\n';
                break;
            }
        }


        std::cout << "SLEEPING..." << '\n';
        usleep(30000000); // 30 seconds
    }
}

double Runner::getTotal(){
    return exec.getBalance();
}
