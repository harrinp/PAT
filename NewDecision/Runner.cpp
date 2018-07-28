#include "../Headers/Decision.hpp"

Runner::Runner(std::string table, std::string longerTable, double takeProfit, double stopLoss) :
    Decider(true, 0),
    exec() {
    this->stopLoss    = stopLoss;
    this->takeProfit  = takeProfit;
    this->table       = table;
    this->longerTable = longerTable;
}

void Runner::run() {
    int    lastHourly = 0;
    Pos    pos        = NOTHING;
    double bal        = 0;

    while (true) {
        std::cout << "Current time: " << std::time(0) << '\n';
        bal = exec.getBalance();
        sql::Statement *stmt = con->createStatement();
        sql::ResultSet *res  = stmt->executeQuery("SELECT date, closeAsk, closeBid FROM " + DATABASE_NAME + "." + table + " ORDER BY DATE DESC LIMIT 1");
        res->next();
        Price p = Price(res->getInt("date"), res->getDouble("closeAsk"), res->getDouble("closeBid"));

        std::cout << "Last data   : " << res->getInt("date") << '\n';

        delete res;
        delete stmt;

        exec.trades = Trade::translateTrades(exec.getTradesJson());

        if(exec.trades.size() == 0){
            std::cout << "No trades" << '\n';
        }

        for (int i = 0; i < (int)exec.trades.size(); i++) {
            if (exec.trades[i].profit < -1 *stopLoss * exec.trades[i].units * p.ask || exec.trades[i].profit > takeProfit * exec.trades[i].units * p.ask) {
                if (exec.trades[i].type == SHORT) {
                    std::cerr << "ending short" << '\n';
                    exec.buy(exec.trades[i].units, "EUR_USD");
                }else if (exec.trades[i].type == LONG) {
                    std::cout << "ending long" << '\n';
                    exec.sell(exec.trades[i].units, "EUR_USD");
                }
            }
        }


        stmt = con->createStatement();
        res  = stmt->executeQuery("SELECT date, closeAsk, closeBid FROM " + DATABASE_NAME + "." + longerTable + " ORDER BY DATE DESC LIMIT 1");
        res->next();
        if (res->getInt("date") != lastHourly) {

            lastHourly = res->getInt("date");
            pos        = decide(res->getInt("date"));

            switch (pos) {
            case LONGING:
                exec.sell(bal * .01 * 50, "EUR_USD");
                std::cout << "Selling " << bal * .01 * 50 << '\n';
                break;

            case SHORTING:
                exec.buy(bal * .01 * 50, "EUR_USD");
                std::cout << "Buying " << bal * .01 * 50 << '\n';
                break;

            case NOTHING:
                std::cout << "Holding " << '\n';
                break;
            }
        }


        std::cout << "SLEEPING..." << '\n';
        usleep(30000000); // 30 seconds
        delete res;
        delete stmt;
    }
}
