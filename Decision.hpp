#ifndef DECISION_HPP
#define DECISION_HPP

#include <mysql_connection.h>
#include <mysql/mysql.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <unistd.h>
#include "TestExec.hpp"

class Decision {
public:
    Decision(bool, std::string, std::string);
    Decision(bool, std::string, std::string, double, double);
    double decide();
    void realityRun();

    enum Position { BUY, SELL, HOLD };

    double testDecide(Price, Position);

    BackExecutor back;

    sql::Driver *driver;
    sql::Connection *con;

private:
    bool testing;
    double results[2];
    double MACD[2];
    double EMA12[10];
    double EMA26[10];
    double longPrices[10];

    double stopLoss;
    double takeProfit;

    std::string table;
    std::string longerTable;

    Position evalMACD();
    Position evalEMA();
    void fillResults(int, std::string);
    void fillLongPrices(int);
};

#endif
