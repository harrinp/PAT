#ifndef FULLBAR_HPP
#define FULLBAR_HPP

#include <unistd.h>
#include <vector>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include "Config.hpp"
#include "Trade.hpp"

#include <sqlite3.h>

/*
    This class is all the data from a bar in our database
*/
class FullBar {
public:
    // Constructors
    FullBar(int date, std::string tableName, bool);
    FullBar(std::string tableName, bool);
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
        double result,
        std::string table,
        bool hasMACD);
    FullBar(
        int date,
        int volume,
        double openBid,
        double closeBid,
        double openAsk,
        double closeAsk,
        std::string table,
        bool hasMACD);

    std::string table;
    bool hasMACD;

    //Data
    unsigned int date;
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

    //Functions
    std::string printableBar();
    static std::vector<FullBar> getBarsBetween(unsigned int, unsigned int, std::string, bool);
    static std::vector<FullBar> getBarsGreater(unsigned int, std::string, bool);
    static std::vector<FullBar> getAnalysisBars(std::string);
    static int putAnalysisBars(std::vector<FullBar>, std::string);
    Price convertToPrice();
};

#endif
