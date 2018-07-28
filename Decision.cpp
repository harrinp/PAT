#include "Headers/Decision.hpp"

int main(int argc, char *argv[]) {
    Decision d = Decision(false, "EUR_USD_M1", "EUR_USD_H1", .0007, .0049);

    if (argc > 1 && strcmp(argv[1], "-t") == 0) {
        Decision d = Decision(true, "EUR_USD_M1", "EUR_USD_H1", .0007, .0049);
    }

    d.decide();
    return EXIT_SUCCESS;
}

Decision::Decision(bool isTesting, std::string table, std::string longerTable) :
    exec(),
    back(6000.0, table, 50.0),
    testing(isTesting),
    table(table),
    longerTable(longerTable) {
    driver     = get_driver_instance();
    con        = driver->connect("tcp://127.0.0.1:3306", "root", "");
    takeProfit = 10;
    stopLoss   = 10;
}

Decision::Decision(bool isTesting, std::string table, std::string longerTable, double takeProfit, double stopLoss) :
    back(6000.0, table, 50.0),
    testing(isTesting),
    stopLoss(stopLoss),
    exec(),
    takeProfit(takeProfit),
    table(table),
    longerTable(longerTable) {
    driver = get_driver_instance();
    con    = driver->connect("tcp://127.0.0.1:3306", "root", "");
}

double Decision::decide() {
    Decision::Position pos = HOLD;
    Price  p      = Price(0, 0, 0);
    double amount = 0;

    if (testing) {
        amount = testDecide(p, pos);
    }else {
        realityRun();
        std::cout << "Heyo" << std::endl;
    }
    delete con;
    return amount;
}

Decision::Position Decision::evalMACD() {
    if (results[0] >= 0 && results[1] < 0) {    // && MACD[1] < 0){
        return BUY;
    }else if (results[0] < 0 && results[1] >= 0) {
        return SELL;
    }else {
        return HOLD;
    }
}

Decision::Position Decision::evalEMA() {
    //bool alreadySignaled = false;
    double diff = EMA12[9] - EMA26[9];

    if (((EMA12[8] - EMA26[8] < 0 && diff < 0) || (EMA12[8] - EMA26[8] > 0 && diff > 0))) {
        return HOLD;
    }else if (EMA12[9] > EMA26[9]) {
        return BUY;
    }else {
        return SELL;
    }
}

void Decision::fillResults(int date, std::string col) {
    sql::Statement *stmt = con->createStatement();
    sql::ResultSet *res  = stmt->executeQuery("SELECT " + col + " FROM " + DATABASE_NAME + ".MACD_" + longerTable + " WHERE date = " + std::to_string(date));

    res->next();
    if (col == "MACD") {
        MACD[0] = MACD[1];
        MACD[1] = res->getDouble(col);
    }else {
        results[0] = results[1];
        results[1] = res->getDouble(col);
    }
    delete res;
    delete stmt;
}

void Decision::fillLongPrices(int date) {
    sql::Statement *stmt = con->createStatement();
    sql::ResultSet *res  = stmt->executeQuery("SELECT * FROM " + DATABASE_NAME + ".MACD_" + longerTable + " WHERE date = " + std::to_string(date));

    res->next();
    for (int i = 0; i < 9; i++) {
        EMA12[i] = EMA12[i + 1];
    }
    EMA12[9] = res->getDouble("EMA12");

    for (int i = 0; i < 9; i++) {
        EMA26[i] = EMA26[i + 1];
    }
    EMA26[9] = res->getDouble("EMA26");
    delete res;
    delete stmt;
}

void closeAllReal() {
}

void Decision::realityRun() {
    int      lastHourly = 0;
    Position pos        = HOLD;
    double   bal        = 0;

    while (true) {
        bal = exec.getBalance();
        sql::Statement *stmt = con->createStatement();
        sql::ResultSet *res  = stmt->executeQuery("SELECT date, closeAsk, closeBid FROM " + DATABASE_NAME + "." + table + " ORDER BY DATE DESC LIMIT 1");
        delete stmt;
        stmt = con->createStatement();
        res->next();
        Price p = Price(res->getInt("date"), res->getDouble("closeAsk"), res->getDouble("closeBid"));

        json j = exec.getTradesJson();
        exec.trades = Trade::translateTrades(j);

        for (int i = 0; i < exec.trades.size(); i++) {
            if (exec.trades[i].profit <-1 *stopLoss *exec.trades[i].units *p.ask || exec.trades[i].profit> takeProfit * exec.trades[i].units * p.ask) {
                if (exec.trades[i].type == SHORT) {
                    std::cerr << "ending short" << '\n';
                    exec.buy(exec.trades[i].units, "EUR_USD");
                }else if (exec.trades[i].type == LONG) {
                    std::cout << "ending long" << '\n';
                    exec.sell(exec.trades[i].units, "EUR_USD");
                }
            }
        }
        delete res;
        delete stmt;
        stmt = con->createStatement();
        res  = stmt->executeQuery("SELECT date, closeAsk, closeBid FROM " + DATABASE_NAME + "." + longerTable + " ORDER BY DATE DESC LIMIT 1");
        res->next();
        if (res->getInt("date") != lastHourly) {
            std::cout << "HERE" << '\n';
            lastHourly = res->getInt("date");
            pos        = evalMACD();
            switch (pos) {
            case BUY:
                exec.sell(bal * .01, "EUR_USD");
                std::cout << "Selling " << bal * .01 << '\n';
                break;

            case SELL:
                exec.buy(bal * .01, "EUR_USD");
                std::cout << "Buying " << bal * .01 << '\n';
                break;

            case HOLD:
                std::cout << "Holding " << bal * .01 << '\n';
                break;
            }
        }

        auto t = std::time(0);

        std::cout << "Current time: " << t << '\n';
        std::cout << "SLEEPING..." << '\n';
        usleep(30000000); // 30 seconds
        delete res;
        delete stmt;
    }
}

double Decision::testDecide(Price p, Position pos) {
    sql::Statement *stmt    = con->createStatement();
    sql::ResultSet *res     = stmt->executeQuery("SELECT date, closeAsk, closeBid FROM " + DATABASE_NAME + "." + table + " WHERE date > 1187902800");
    int             date    = 0;
    int             counter = 0;
    while (res->next() && counter < 1 * 365 * 1440) {
        if (counter % 1440 == 0) {
            std::cout << "day " << counter / 1440 << "\n";
        }
        date = res->getInt("date");

        p = Price(res->getInt("date"), res->getDouble("closeAsk"), res->getDouble("closeBid"));

        for (int i = 0; i < back.trades.size(); i++) {
            if (back.trades[i].calcProfit(p) < -1 * stopLoss * back.trades[i].units * p.ask || back.trades[i].calcProfit(p) > takeProfit * back.trades[i].units * p.ask) {
                back.closeTrade(p, i);
            }
        }

        if (date % 3600 == 0) {
            fillLongPrices(date);
            fillResults(date, "result");
            fillResults(date, "MACD");
            //pos = evalEMA();
            pos = evalMACD();
            switch (pos) {
            case BUY:
                back.closeAll(p);
                back.makeTrade(SHORT, (int)(0.01 * back.USD * 50), p);
                break;

            case SELL:
                back.closeAll(p);
                back.makeTrade(LONG, (int)(0.01 * back.USD * 50), p);
                break;

            case HOLD: break;
            }
        }

        counter++;
    }
    back.closeAll(Price(res->getInt("date"), res->getDouble("closeAsk"), res->getDouble("closeBid")));
    back.reportTrades();
    delete res;
    delete stmt;
    return back.USD;
}
