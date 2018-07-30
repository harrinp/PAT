#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

//#include "Decision.hpp"
#include <string>
#include <unistd.h>
#include <curl/curl.h>
#include <string>
#include "nlohmann/json.hpp"
#include "Trade.hpp"
#include "Config.hpp"


// for convenience
using json = nlohmann::json;

class Executor {
public:

    Executor();

    std::vector<Trade> trades;

    static size_t responseWriter(void *contents, size_t size, size_t nmemb, std::string *s);
    static json post(std::string, json, std::vector<std::string>);
    static json get(std::string path);

    static int buyOrSell(int, std::string);
    bool sell(int, std::string);
    bool buy(int, std::string);

    json getTradesJson();
    double getProfit();
    double getBalance();
};

#endif
