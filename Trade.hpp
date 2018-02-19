#ifndef TRADE_HPP
#define TRADE_HPP

#include <iostream>
#include <string>
#include <unistd.h>
#include "nlohmann/json.hpp"

//#include "Trade.cpp"

using json = nlohmann::json;

enum TradeType { LONG, SHORT };

class Price {
public:
    Price(int, double, double);
    Price(std::string, int);
    double bid;
    double ask;
    int date;
};

class Trade {
public:
    Trade(TradeType, int, Price, double, std::string);

    bool open;
    int units;
    double profit;
    int closeDate;
    double marginUsed;
    double leverage;

    double totalValueAtClose;
    std::string instrument;

    Price initialPrice;
    Price finalPrice;
    double calcProfit(Price price);

    static std::vector<Trade> translateTrades(json);

    double close(Price p);

    std::string tradeAsString();

    TradeType type;
};


#endif
