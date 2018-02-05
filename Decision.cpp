#include "Decision.hpp"

int main() {
    Decision d = Decision(true, "EUR_USD_M1", "EUR_USD_H1", .007, .049);

    d.decide();
    return EXIT_SUCCESS;
}

Decision::Decision(bool isTesting, std::string table, std::string longerTable) :
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
    takeProfit(takeProfit),
    table(table),
    longerTable(longerTable) {
    driver = get_driver_instance();
    con    = driver->connect("tcp://127.0.0.1:3306", "root", "");
}

double Decision::decide() {
    Decision::Position pos = HOLD;
    Price p = Price(0, 0, 0);
    double amount = 0;

    if (testing) {
        amount = testDecide(p, pos);
    }
    else {
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
    sql::ResultSet *res  = stmt->executeQuery("SELECT " + col + " FROM quotesdb.MACD_" + longerTable + " WHERE date = " + std::to_string(date));

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
    sql::ResultSet *res  = stmt->executeQuery("SELECT * FROM quotesdb.MACD_" + longerTable + " WHERE date = " + std::to_string(date));

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

void Decision::realityRun(){
    while (true) {
        int t = time(0);
        std::cout << t << '\n';
        sql::Statement *stmt    = con->createStatement();
        sql::ResultSet *res = stmt->executeQuery("SELECT date, closeAsk, closeBid FROM quotesdb." + table + " WHERE date < " + std::to_string(t) + " LIMIT 1");
        delete stmt;
        stmt = con->createStatement();

        while(res->next()){
            //TODO: Put in the code from testDecide
            Price p = Price(res->getInt("date"), res->getDouble("closeAsk"), res->getDouble("closeBid"));

        }
        usleep(5000000);    // 5 Seconds
        delete res;
        delete stmt;
    }
}

double Decision::testDecide(Price p, Position pos){
    sql::Statement *stmt    = con->createStatement();
    sql::ResultSet *res     = stmt->executeQuery("SELECT date, closeAsk, closeBid FROM quotesdb." + table + " WHERE date > 1187902800");
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
