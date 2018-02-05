#include "Trade.hpp"
#include <string>

Trade _convertTrade(json j){
    TradeType type = LONG;
    std::cout << "Current units:" << '\n';
    if (std::stoi(j.at("currentUnits").get<std::string>()) < 0){
        type = SHORT;
    }
    std::cout << "HI" << '\n';
    Trade t = Trade(
        type,       //TODO: change this to an inline-if (Whatever those are called I forget)
        abs(std::stoi(j.at("currentUnits").get<std::string>())),
        Price(
            std::stoi(j.at("openTime").get<std::string>()),
            std::stod(j.at("price").get<std::string>()),
            std::stod(j.at("price").get<std::string>())
        ),
        50.0,    // This is the amount of leverage, assumed to be 50 <- this needs work
        j.at("instrument").get<std::string>()
    );
    std::cout << "HI" << '\n';
    t.profit = std::stod(j.at("unrealizedPL").get<std::string>());
    return t;
}

std::vector<Trade> Trade::translateTrades(json response){

    std::vector<Trade> trades;

    json j = response.at("trades").get<json>();

    for (auto& element : j) {
        std::cout << "HI" << '\n';
        trades.push_back(_convertTrade(element));

        std::cout << trades.back().tradeAsString() << '\n';
    }
    return trades;
}

std::string Trade::tradeAsString(){
    std::string s = "Trade : ";
    s.append(type == LONG ? "LONG\n" : "SHORT\n");
    s.append("Time: ");
    s.append(std::to_string(initialPrice.date));
    s.append("\nProfit: ");
    s.append(std::to_string(profit));
    s.append("\n");
    return s;
}


Trade::Trade(TradeType type, int units, Price price, double leverage, std::string instrument) : type(type), units(units), initialPrice(price), leverage(leverage), instrument(instrument), finalPrice(Price(0, 0.0, 0.0)) {
    open = true;
    type == LONG ? marginUsed = units * price.ask / leverage : units * price.bid / leverage;
}

/*
 *   Returns the unrealized profit of the position in USD

    USED IN TESTING ONLY!
    IN PRODUCTION -> refresh list of trades and use broker-given unrealized profit/loss
 */
double Trade::calcProfit(Price price) {
    profit     = 0.0;
    finalPrice = price;

    if (type == LONG) {
        profit = (double)units * (price.bid - initialPrice.ask);
    }else  {
        profit = (double)units * (initialPrice.bid - price.ask);
    }

    return profit;
}

double Trade::close(Price p) {
    double useMe = calcProfit(p);
    open      = false;
    closeDate = p.date;
    return useMe + marginUsed;
}

Price::Price(int date, double ask, double bid) : date(date), bid(bid), ask(ask) {
}
