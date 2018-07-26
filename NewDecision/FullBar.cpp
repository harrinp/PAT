#include "../Headers/FullBar.hpp"

FullBar::FullBar(int date, std::string tableName) {
    sql::Driver *    driver;
    sql::Connection *con;
    driver = get_driver_instance();
    con    = driver->connect(URL, USER, PASSWORD);
    sql::Statement *stmt = con->createStatement();
    sql::ResultSet *res  = stmt->executeQuery("SELECT * FROM " + DATABASE_NAME + tableName + " WHERE date = " + std::to_string(date));
    delete con;

    res->next();
    this->date = date;
    openBid    = res->getDouble("openBid");
    closeBid   = res->getDouble("closeBid");
    openAsk    = res->getDouble("openAsk");
    closeAsk   = res->getDouble("closeAsk");
    volume     = res->getInt("volume");

    delete stmt;
    delete res;
    stmt = con->createStatement();
    res  = stmt->executeQuery("SELECT * FROM " + DATABASE_NAME + ".MACD_" + tableName + " WHERE date = " + std::to_string(date));
}

FullBar::FullBar(
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
        double result) : date(date), openBid(openBid), closeBid(closeBid), openAsk(openAsk), volume(volume), EMA26(EMA26), EMA12(EMA12), MACD(MACD), sign(sign), result(result) {
}
