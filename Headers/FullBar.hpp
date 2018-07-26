#ifndef FULLBAR_HPP
#define FULLBAR_HPP

#include <unistd.h>
#include <vector>
#include <mysql_connection.h>
#include <mysql/mysql.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include "../Headers/Config.hpp"

/*
    This class is all the data from a bar of data in our mysql database
*/
class FullBar {
public:
    FullBar (int date, std::string tableName);
    virtual ~FullBar ();
    FullBar(
        int date, 
        int volume, 
        double openBid, 
        double closeBid, 
        double openAsk, 
        double closeAsk, 
        double EMA26, 
        double EMA12, 
        double MACD, 
        double sign, 
        double result);
    int date;
    double openBid;
    double closeBid;
    double openAsk;
    double closeAsk;
    int volume;
    //MACD Table
    double EMA26;
    double EMA12;
    double MACD;
    double sign;
    double result;

};

#endif
