#ifndef DECISION_HPP
#define DECISION_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <vector>

#include "Trade.hpp"
#include "TestExec.hpp"
#include "Executor.hpp"
#include "Config.hpp"
#include "FullBar.hpp"

/*

    NEW STUFF

*/

class Decider {
public:

    Decider(bool, unsigned int);
    virtual ~Decider() = default;

    enum Pos { LONGING, SHORTING, NOTHING };

    bool testing;

    Pos decide(unsigned int);

    Pos evalMACD();
    Pos evalEMA();
    void fillResults(int, std::string);
    void fillLongPrices(int);
    //These use a FullBar instead of their own sql calls for speed
    Pos decide(FullBar *);
    void fillResults(FullBar *, std::string);
    void fillLongPrices(FullBar *);

    virtual double getTotal() = 0;

    virtual void run() = 0;

protected:
    unsigned int time;

    //Percentages
    double stopLoss;
    double takeProfit;

    //Database connection:
    sql::Driver *driver;
    sql::Connection *con;

    //Database access parameters
    std::string table;
    std::string longerTable;

    //For doing MACD analysis (eventually should be broken into its own class)
    double results[2];
    double MACD[2];
    double EMA12[10];
    double EMA26[10];

};

class Tester: public Decider {
public:
    //virtual ~Tester(){};
    Tester(std::string, std::string, double, double);
    Tester(std::string, std::string, double, double, std::vector <FullBar> *, std::vector <FullBar> *);
    void run();
    void runMany();
    void closeProfitableOrNotTrades(Price);
    double getTotal();

    bool verbose;

    std::vector <FullBar> * minutes;
    std::vector <FullBar> * hours;
protected:


    BackExecutor back;
};

class Runner: public Decider {
public:
    Runner(std::string, std::string, std::string, double, double);
    void run();
    double getTotal();
protected:
    std::string orderItemName;
    Executor exec;
    int lastHourlyDate;
};

#endif
