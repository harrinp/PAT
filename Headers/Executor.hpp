#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

//#include "Decision.hpp"
#include <string>
#include <unistd.h>
#include <curl/curl.h>
#include "Trade.hpp"


// for convenience
using json = nlohmann::json;

class Executor {
public:

    Executor();

    std::vector<Trade> trades;

    double getProfit();
    static json post(std::string, json, std::vector<std::string>);
    static void test();
    bool buy(int, std::string);
    static int buyOrSell(int, std::string);
    bool sell(int, std::string);
    static size_t responseWriter(void *contents, size_t size, size_t nmemb, std::string *s);
    json getTradesJson();
    json get(std::string path);
    double getBalance();
};

#endif
