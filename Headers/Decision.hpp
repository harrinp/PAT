#ifndef DECISION_HPP
#define DECISION_HPP

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
    void run();
    void closeProfitableOrNotTrades(Price);

protected:
    BackExecutor back;
};

class Runner: public Decider {
public:
    //virtual ~Decisioner(){};
    Runner(std::string, std::string, double, double);
    void run();
protected:
    Executor exec;
};

#endif
