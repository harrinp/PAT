#include "Headers/TestExec.hpp"

BackExecutor::BackExecutor(double money, std::string table, double leverage) : USD(money), table(table), leverage(leverage) {
    //driver = get_driver_instance();
    //con = driver->connect("tcp://127.0.0.1:3306", "root", "");
    counter = 0;
    margin  = 0;
}

/*
 *   To be called after all trades have been closed. Prints a line by line print out of trades
 */
void BackExecutor::reportTrades() {
    //std::cout << oldTrades[0].initialPrice.date << '\n';
    for (int i = 0; i < oldTrades.size(); i++) {
        std::cout << (oldTrades[i].type == LONG ? "LONG," : "SHORT,") <<
            oldTrades[i].initialPrice.date << "," <<
            oldTrades[i].closeDate << "," <<
            oldTrades[i].initialPrice.ask << "," <<
            oldTrades[i].initialPrice.bid << "," <<
            oldTrades[i].finalPrice.ask << "," <<
            oldTrades[i].finalPrice.bid << "," <<
            oldTrades[i].units << "," <<
            oldTrades[i].profit << "," <<
            oldTrades[i].totalValueAtClose << '\n';
    }
}

void BackExecutor::closeAll(Price p) {
    for (int i = 0; i < trades.size(); i++) {
        USD += trades[i].close(p);
        oldTrades.push_back(trades[i]);
        trades.erase(trades.begin() + i);
        oldTrades.back().totalValueAtClose = USD + totalProfit(p);
        i--;
    }
}

/*
 *   Returns the unrealized profit of all positions in USD
 */
double BackExecutor::totalProfit(Price p) {
    double profit = 0.0;

    for (int i = 0; i < trades.size(); i++) {
        profit += trades[i].calcProfit(p);
    }
    return profit;
}

void BackExecutor::makeTrade(TradeType type, int units, Price price) {
    Trade t = Trade(type, units, price, leverage, table);

    trades.push_back(t);
    //double p = type == LONG ? price.ask : price.bid;
    USD -= t.marginUsed;
}

void BackExecutor::closeTrade(Price p, int index) {
    USD += trades[index].close(p);
    oldTrades.push_back(trades[index]);
    trades.erase(trades.begin() + index);
    oldTrades.back().totalValueAtClose = USD + totalProfit(p);
}

Price::Price(std::string table, int date) {
    sql::Driver *    driver = get_driver_instance();
    sql::Connection *con    = driver->connect("tcp://127.0.0.1:3306", "root", "");
    sql::Statement * stmt   = con->createStatement();
    std::string      query  = "SELECT * FROM quotesdb." + table + " WHERE  date = " + std::to_string(date);
    sql::ResultSet * res    = stmt->executeQuery(query);
    res->next();
    bid  = res->getDouble("closeBid");
    ask  = res->getDouble("closeAsk");
    date = date;
    delete con;
}
