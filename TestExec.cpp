#include "Headers/TestExec.hpp"

/*      
 *      This is the 'broker' that the tester communicates with
 *      This class holds the current trades and calculates profits
 *      This class provides seperation between strategy and implementation
 */


/*
 *      Constructor
 *
 *      leverage  : amount we can multiply our trade by
 *      USD/Money : total money starting with
 *      Table     : ex: EUR_USD, USD_CAD
 */
BackExecutor::BackExecutor(double money, std::string table, double leverage) : USD(money), leverage(leverage), numTrades(0), table(table) {
    //driver = get_driver_instance();
    //con = driver->connect("tcp://127.0.0.1:3306", "root", "");
    counter = 0;
    margin  = 0;
}


/*
 *   To be called after all trades have been closed. 
 *   Prints a line by line print out of trades
 *   Output is in .csv format so it can be piped into a file for processing
 */
void BackExecutor::reportTrades() {
    //std::cout << oldTrades[0].initialPrice.date << '\n';
    for (int i = 0; i < (int)oldTrades.size(); i++) {
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

/*
 *      Closes all open trades
 */
void BackExecutor::closeAll(Price p) {
    for (int i = 0; i < (int)trades.size(); i++) {
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

    for (int i = 0; i < (int)trades.size(); i++) {
        profit += trades[i].calcProfit(p);
    }
    return profit;
}

/*
 *      Creates a trade for the given amount of units and at the current price object
 *
 *      Relies on the caller to provide the correct price object, as this is how time is told
 */
void BackExecutor::makeTrade(TradeType type, int units, Price price) {
    Trade t = Trade(type, units, price, leverage, table);
    numTrades++;
    trades.push_back(t);
    //double p = type == LONG ? price.ask : price.bid;
    USD -= t.marginUsed;
}

/*
 *      Closes a trade and moves it to the old trades array
 *
 *      Adds the profit and margin amount spent back into USD
 */
void BackExecutor::closeTrade(Price p, int index) {
    USD += trades[index].close(p);
    oldTrades.push_back(trades[index]);
    trades.erase(trades.begin() + index);
    oldTrades.back().totalValueAtClose = USD;
}

