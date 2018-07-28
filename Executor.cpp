#include "Headers/Executor.hpp"
#include "Headers/QuotesDB.hpp"
#include "Headers/DataBase.hpp"
#include "Headers/OandaAPI.hpp"
using namespace Poco::Net;
using namespace Poco;

// int main(int argc, char const *argv[]) {
//     Executor e;
//     json j = e.getTradesJson();
//     std::vector<Trade> trades = Trade::translateTrades(j);
//
//     return 0;
// }

/*
 *      called internally by default
 *      can be called externally if the two convenience functions (buy() and sell()) are not preferred
 */
int Executor::buyOrSell(int units, std::string instrument) {
    json j = {
        { "order",        {
                    { "units",        std::to_string(units) },
                    { "instrument",   instrument },
                    { "timeInForce",  "FOK" },
                    { "type",         "MARKET" },
                    { "positionFill", "DEFAULT" }
                } },
    };

    std::vector <std::string> v;

    v.push_back("Accept-Datetime-Format: UNIX");
    v.push_back("Authorization: Bearer " + ACCESS_TOKEN);
    v.push_back("Content-Type: application/json");

    Executor::post(std::string("/v3/accounts/" + ACCOUNT_ID + "/orders"), j, v);
    return 1;
}

/*
 *      Constructor
 */
Executor::Executor() {

}

/*
 *      This function is passed through curl and handles the writing of the data into the string
 *      No JSON parsing is done in this function
 */
size_t Executor::responseWriter(void *contents, size_t size, size_t nmemb, std::string *s) {
    size_t newLength = size * nmemb;
    size_t oldLength = s->size();

    try
    {
        s->resize(oldLength + newLength);
    }
    catch (std::bad_alloc &e)
    {
        //handle memory problem
        printf("shit\n");
    }

    std::copy((char *)contents, (char *)contents + newLength, s->begin() + oldLength);
    return size * nmemb;
}

/*
 *      Simple curl post request
 *      uses above responseWriter()
 */
json Executor::post(std::string path, json j, std::vector <std::string> v) {
    std::string body = j.dump();
    CURL *      curl = curl_easy_init();
    std::string s;  // Used by writing function

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_URL, std::string("https://api-fxpractice.oanda.com" + path).c_str());

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, v[0].c_str());
    headers = curl_slist_append(headers, v[1].c_str());
    headers = curl_slist_append(headers, v[2].c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

    //Sets up the writing function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseWriter);   // Our function defined above will be called to handle the response
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);                  

    // Post method
    CURLcode ret = curl_easy_perform(curl);

    // check for error during post
    if (ret != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(ret));
        std::cout << ret << '\n';
    }

    return json::parse(s);
}

/*
 *      Convenience wrapper for buyOrSell()
 */
bool Executor::buy(int units, std::string pair) {
    Executor::buyOrSell(units, pair);
    return 0;
}

/*
 *      Convenience wrapper for buyOrSell()
 */
bool Executor::sell(int units, std::string pair) {
    Executor::buyOrSell(-1 * units, pair);
    return 0;
}

/*
 *      Testing function not to be used at run time
 */
void Executor::test() {
    qdb::OandaAPI conn("practice");
    std::string   x("EUR_USD_D");
    std::string   endpoint("/v3/accounts/" + ACCOUNT_ID + "/instruments");
    //std::cout << conn.request(endpoint) << std::endl;

    json j;

    j = json::parse(conn.request(endpoint));
    std::cout << j.dump(4) << std::endl;
}

/*
 *      Returns total profit from current trades
 *      Caller must update trades array BEFORE calling
 *      This function does not update trades array
 *      Trades array can be updated using the functions in the Trade class
 */
double Executor::getProfit() {
    double profit = 0;

    for (size_t i = 0; i < trades.size(); i++) {
        profit += trades[i].profit;
    }
    return profit;
}

/*
 *      TODO: Switch this to curl
 */
double Executor::getBalance(){
    std::string x;
    qdb::OandaAPI oanda = qdb::OandaAPI("practice");
    x = oanda.request(std::string("/v3/accounts/") + ACCOUNT_ID + std::string("/summary"));
    json j = json::parse(x);
    j = j.at("account").get<json>();
    double y = std::stod(j.at("balance").get<std::string>());
    return y;
}

/*
 *      TODO: Switch this to curl
 */
json Executor::getTradesJson() {
    std::string x;
    qdb::OandaAPI oanda = qdb::OandaAPI("practice");
    x = oanda.request(std::string("/v3/accounts/") + ACCOUNT_ID + std::string("/openTrades"));
    json j = json::parse(x);
    return j;
}
