#include "../Headers/Decision.hpp"

Tester::Tester(std::string table, std::string longerTable, double takeProfit, double stopLoss) :
    Decider(true, 0),
    back(6000.0, table, 50.0) {
    this->stopLoss    = stopLoss;
    this->takeProfit  = takeProfit;
    this->table       = table;
    this->longerTable = longerTable;
}

void Tester::closeProfitableOrNotTrades(Price p){
     for (int i = 0; i < (int)back.trades.size(); i++) {
            if (
                back.trades[i].calcProfit(p) < -1 * stopLoss * back.trades[i].units * p.ask || 
                back.trades[i].calcProfit(p) > takeProfit * back.trades[i].units * p.ask) {
                back.closeTrade(p, i);
            }
        }


}

void Tester::run() {

    Price p = Price(0,0,0);
    Pos pos = NOTHING;

    sql::Statement *stmt    = con->createStatement();
    sql::ResultSet *res     = stmt->executeQuery("SELECT date, closeAsk, closeBid FROM " + DATABASE_NAME + "." + table + " WHERE date > 1187902800");

    int             date    = 0;
    int             counter = 0;
    while (res->next() && counter < 1 * 365 * 1440) {
        if (counter % 1440 == 0) {
            std::cout << "day " << counter / 1440 << " --- Profit: " << back.USD << "\n";
        }
date = res->getInt("date");
        p = Price(res->getInt("date"), res->getDouble("closeAsk"), res->getDouble("closeBid"));

        closeProfitableOrNotTrades(p);

        if (date % 3600 == 0) {
            fillLongPrices(date);
            fillResults(date, "result");
            fillResults(date, "MACD");
            //pos = evalEMA();
            pos = evalMACD();
            switch (pos) {
            case LONGING:
                //back.closeAll(p);
                back.makeTrade(SHORT, (int)(0.01 * back.USD * 50), p);
                break;

            case SHORTING:
                //back.closeAll(p);
                back.makeTrade(LONG, (int)(0.01 * back.USD * 50), p);
                break;

            case NOTHING: break;
            }
        }

        counter++;
    }
    back.closeAll(Price(res->getInt("date"), res->getDouble("closeAsk"), res->getDouble("closeBid")));
    back.reportTrades();
    delete res;
    delete stmt;
}
