#include "Headers/Trade.hpp"
//#include <string>

/*
 *      converts json from oanda into a single Trade object
 */
Trade _convertTrade(json j){
    TradeType type = LONG;
    //std::cout << "Current units:" << '\n';
    if (std::stoi(j.at("currentUnits").get<std::string>()) < 0){
        type = SHORT;
    }
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
    t.profit = std::stod(j.at("unrealizedPL").get<std::string>());
    return t;
}


/*
 *      Creates an array of Trade objects using a response from the oanda server
 *      Takes the FULL json response from the executor, not just the `at("trades")` part
 *      Calls _convertTrade at least 1 time to do this
 */
std::vector<Trade> Trade::translateTrades(json response){
    std::cerr << "Translating trades:" << '\n';
    std::vector<Trade> trades;

    json j = response.at("trades").get<json>();
    for (auto& element : j) {
        trades.push_back(_convertTrade(element));
        std::cout << trades.back().tradeAsString() << '\n';
    }
    return trades;
}

/*
 *      Printable form of a Trade object's information
 */
std::string Trade::tradeAsString(){
    std::string s = "Trade : ";
    s.append(type == LONG ? "LONG\n" : "SHORT\n");
    s.append("Current Units: ");
    s.append(std::to_string(units));
    s.append("\nTime: ");
    s.append(std::to_string(initialPrice.date));
    s.append("\nProfit: ");
    s.append(std::to_string(profit));
    s.append("\n");
    return s;
}

/*
 *      Constructor for a trade
 *      units      : number of base currency being traded
 *      Instrument : Synonym for table, ex: "EUR_USD"
 */
Trade::Trade(TradeType type, int units, Price price, double leverage, std::string instrument) : type(type), units(units), initialPrice(price), leverage(leverage), instrument(instrument), finalPrice(Price(0, 0.0, 0.0)) {
    open = true;
    type == LONG ? marginUsed = units * price.ask / leverage : marginUsed = units * price.bid / leverage;
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

/*
 *      Close this trade
 *      returns profit plus the amount spent on the trade
 */
double Trade::close(Price p) {
    double useMe = calcProfit(p);
    open      = false;
    closeDate = p.date;
    return useMe + marginUsed;
}

/*
 *      Constructor for a price object
 *
 *      Trade objects have initialPrice and finalPrice objects to calculate profit
 */
Price::Price(int date, double ask, double bid) : bid(bid), ask(ask), date(date){
}

Price::Price(std::string table, int date) {
    // sql::Driver *    driver = get_driver_instance();
    // sql::Connection *con    = driver->connect("tcp://127.0.0.1:3306", "root", "");
    // sql::Statement * stmt   = con->createStatement();
    // std::string      query  = "SELECT * FROM quotesdb." + table + " WHERE  date = " + std::to_string(date);
    // sql::ResultSet * res    = stmt->executeQuery(query);
    // res->next();
    // bid  = res->getDouble("closeBid");
    // ask  = res->getDouble("closeAsk");
    // delete con;
    // delete res;
}
