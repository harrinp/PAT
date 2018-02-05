#include "Analysis.hpp"

int main() {
    Analysis a = Analysis();

    //a.initializeMACDTable("MACD_EUR_USD_H1", "EUR_USD_H1");
    //a.calcMACD("MACD_EUR_USD_H1", "EUR_USD_H1");
    //a.initializeMACDTable("MACD_EUR_USD_M1", "EUR_USD_M1");
    //a.calcMACD("MACD_EUR_USD_M1", "EUR_USD_M1");

    a.analyze();

    delete a.con;
    return EXIT_SUCCESS;
}

Analysis::Analysis() : conn("practice"){
    driver = get_driver_instance();
    con    = driver->connect("tcp://127.0.0.1:3306", "root", "");
    //qdb::OandaAPI ;
}

void Analysis::analyze() {
    while (true) {
        std::cout << "GET QUOTES:" << '\n';
        conn.updateAllTabs("QuotesDB");

        std::cout << "ANALYZE H AND M" << '\n';
        calcMACD("MACD_EUR_USD_M1", "EUR_USD_M1");
        std::cout << "DONE M" << '\n';
        calcMACD("MACD_EUR_USD_H1", "EUR_USD_H1");
        std::cout << "DONE H" << '\n';
        std::cout << "Sleeping..." << '\n';
        usleep(30000000);   //30 Seconds
    }
}

int Analysis::calcMACD(std::string result, std::string data) {
//    std::cout << "RUNNING" << '\n';
    sql::Statement *stmt    = con->createStatement();
    sql::ResultSet *resDate = stmt->executeQuery("SELECT date FROM quotesdb." + result + " ORDER BY date DESC LIMIT 1");
    resDate->next();

    int MACDDate = resDate->getInt(1);

    stmt    = con->createStatement();
    resDate = stmt->executeQuery("SELECT date FROM quotesdb." + data + " ORDER BY date DESC LIMIT 1");
    resDate->next();

    stmt    = con->createStatement();
    resDate = stmt->executeQuery("SELECT * FROM quotesdb." + data + " WHERE date > " + std::to_string(MACDDate));

    sql::PreparedStatement *prep_stmt;
    prep_stmt = con->prepareStatement("INSERT INTO quotesdb." + result + "(date, EMA26, EMA12, MACD, sign, result) VALUES(?, ?, ?, ?, ?, ?)");

    double EMA12    = 0.0;
    double EMA26    = 0.0;
    double sign     = 0.0;
    int    prevDate = MACDDate;

    while (resDate->next()) {
        std::cout << "~";
        int date = resDate->getInt("date");

        EMA12 = EMA(12, 0.0, prevDate, date, data, result, "EMA12", "closeAsk");
        EMA26 = EMA(26, 0.0, prevDate, date, data, result, "EMA26", "closeAsk");

        prep_stmt->setInt(1, date);
        prep_stmt->setDouble(2, EMA26);
        prep_stmt->setDouble(3, EMA12);
        prep_stmt->setDouble(4, EMA12 - EMA26);

        sign = EMA(9, EMA12 - EMA26, prevDate, date, result, result, "sign", "MACD");

        prep_stmt->setDouble(5, sign);
        prep_stmt->setDouble(6, (EMA12 - EMA26) - sign);
        prep_stmt->execute();
        prevDate = date;
        // if (date % 3600 == 0) {
        //     std::cout << date << "," << EMA12 << "," << EMA26 << "," << EMA12 - EMA26 << "," << sign << "," << (EMA12 - EMA26) - sign << '\n';
        // }
    }
    delete resDate;
    delete stmt;
    return 1;
}

double Analysis::EMA(int num, double val, int prevDate, int newDate, std::string dataTableName, std::string resultTableName, std::string ema, std::string newDataField) {
    sql::Statement *stmt = con->createStatement();
    sql::ResultSet *res  = stmt->executeQuery("SELECT * FROM quotesdb." + resultTableName + " WHERE date = " + std::to_string(prevDate));
    res->next();
    double prevEMA = res->getDouble(ema);

    double ask = 0.0;

    if (num != 9) {
        stmt = con->createStatement();
        res  = stmt->executeQuery("SELECT * FROM quotesdb." + dataTableName + " WHERE date = " + std::to_string(newDate));
        res->next();
        ask = res->getDouble(newDataField);
    }else {
        ask = val;
    }


    double mult = 2.0 / (num + 1.0);
    //std::cout << prevEMA << "   " << ask << "   " << (ask - prevEMA) * mult + prevEMA << '\n';
    delete res;
    delete stmt;

    return (ask - prevEMA) * mult + prevEMA;
}

void Analysis::initializeMACDTable(std::string initialize, std::string data) {
    /*
     *  deleting old table
     */

    sql::Statement *stmt = con->createStatement();
    stmt->execute("TRUNCATE quotesdb." + initialize);

    /*
     *  getting the first EMA12 which is the linear average of first 12
     */

    stmt = con->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT * FROM quotesdb." + data + " ORDER BY date ASC LIMIT 35");

    sql::PreparedStatement *prep_stmt;

    prep_stmt = con->prepareStatement("INSERT INTO quotesdb." + initialize + "(date, EMA12) VALUES (?, ?)");

    double asks [35];


    for (int i = 0; i < 12; i++) {
        res->next();
        asks[i] += res->getDouble("closeAsk");
    }
    double ave   = 0.0;
    double count = 0.0;
    for (int i = 11; i >= 0; i--) {
        ave   += asks[i] * (i + 1);
        count += i + 1;
    }
    ave /= count;

    int prevDate = res->getInt("date");

    prep_stmt->setInt(1, prevDate);
    prep_stmt->setDouble(2, ave);
    prep_stmt->execute();

    /*
     *  getting the next 13 EMA12 which are normal EMAs
     */

    double EMA12 = 0;

    for (int i = 0; i < 13; i++) {
        res->next();
        asks[12 + i] = res->getDouble("closeAsk");

        EMA12    = EMA(12, 0.0, prevDate, res->getInt("date"), data, initialize, "EMA12", "closeAsk");
        prevDate = res->getInt("date");
        //std::cout << "DATE:" << prevDate << '\n';

        prep_stmt->setInt(1, prevDate);
        prep_stmt->setDouble(2, EMA12);
        prep_stmt->execute();
    }

    /*
     *  getting the first EMA26 (linear average of first 26) while getting next EMA12 and MACD
     */

    prep_stmt = con->prepareStatement("INSERT INTO quotesdb." + initialize + "(date, EMA26, EMA12, MACD) VALUES (?, ?, ?, ?)");

    res->next();

    double MACDs [9];

    ave   = 0.0;
    count = 0.0;
    for (int i = 24; i >= 0; i--) {
        ave   += asks[i] * (i + 1);
        count += i + 1;
    }
    ave  /= count;
    EMA12 = EMA(12, 0.0, prevDate, res->getInt("date"), data, initialize, "EMA12", "closeAsk");

    prep_stmt->setDouble(3, EMA12);
    prep_stmt->setDouble(2, ave);
    prep_stmt->setInt(1, res->getInt("date"));
    prep_stmt->setDouble(4, EMA12 - ave);
    prep_stmt->execute();
    prevDate = res->getInt("date");
    MACDs[0] = EMA12 - ave;

    double EMA26 = 0.0;

    for (int i = 0; i < 7; i++) {
        res->next();
        EMA12 = EMA(12, 0.0, prevDate, res->getInt("date"), data, initialize, "EMA12", "closeAsk");
        EMA26 = EMA(26, 0.0, prevDate, res->getInt("date"), data, initialize, "EMA26", "closeAsk");
        prep_stmt->setDouble(3, EMA12);
        prep_stmt->setDouble(2, EMA26);
        prep_stmt->setInt(1, res->getInt("date"));
        prep_stmt->setDouble(4, EMA12 - EMA26);
        prep_stmt->execute();
        prevDate     = res->getInt("date");
        MACDs[1 + i] = EMA12 - EMA26;
    }

    res->next();
    EMA12 = EMA(12, 0.0, prevDate, res->getInt("date"), data, initialize, "EMA12", "closeAsk");
    EMA26 = EMA(26, 0.0, prevDate, res->getInt("date"), data, initialize, "EMA26", "closeAsk");

    MACDs[8] = EMA12 - EMA26;

    prep_stmt = con->prepareStatement("INSERT INTO quotesdb." + initialize + "(date, EMA26, EMA12, MACD, sign, result) VALUES(?, ?, ?, ?, ?, ?)");


    prep_stmt->setDouble(3, EMA12);
    prep_stmt->setDouble(2, EMA26);
    prep_stmt->setInt(1, res->getInt("date"));
    prep_stmt->setDouble(4, EMA12 - EMA26);


    double sig = 0.0;
    count = 0.0;
    for (int i = 8; i >= 0; i--) {
        sig   += MACDs[i] * (i + 1);
        count += i + 1;
    }
    sig /= count;
    prep_stmt->setDouble(5, sig);
    prep_stmt->setDouble(6, MACDs[8] - sig);
    prep_stmt->execute();
    prevDate = res->getInt("date");


    delete res;
    delete stmt;
}
