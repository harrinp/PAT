#ifndef FULLBAR_HPP
#define FULLBAR_HPP

#include <unistd.h>
#include <vector>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include "../Headers/Config.hpp"

#include <sqlite3.h>

/*
    This class is all the data from a bar in our database
*/
class FullBar {
public:
    // Constructors
    FullBar(int date, std::string tableName);

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
        std::string table);

    std::string table;

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
    static std::vector<FullBar> getBarsBetween(unsigned int, unsigned int, std::string);
    static std::vector<FullBar> getBarsGreater(unsigned int, std::string);
};

#endif
