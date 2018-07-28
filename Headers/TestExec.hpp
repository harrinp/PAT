#ifndef TESTEXEC_HPP
#define TESTEXEC_HPP

#include <vector>
#include <mysql_connection.h>
#include <mysql/mysql.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <unistd.h>

#include "Trade.hpp"

class BackExecutor {
public:
    double USD;
    int counter;
    double margin;
    double leverage;
    int numTrades;

    BackExecutor(double, std::string, double);
    void closeAll(Price);
    double getBal(int);
    double getBalEUR(int);
    void makeTrade(TradeType type, int units, Price price);

    void closeTrade(Price, int);
    double totalProfit(Price);
    void reportTrades();

    std::vector <Trade> trades;

    sql::Driver *driver;
    sql::Connection *con;

private:
    std::string table;

    std::vector <Trade> oldTrades;

    double getPrice(std::string, int);
};


#endif
