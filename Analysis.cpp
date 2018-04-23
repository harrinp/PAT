#include "Headers/Analysis.hpp"
#include <time.h>

int main(int argc, char *argv[]) {
    Analysis a = Analysis();

    // Checks to see if you are doing initialization
    if (argc > 1 && strcmp(argv[1], "-i") == 0) {
        // I moved initialization together so if you have to stop it you can
        // restart with no problems
        a.initializeMACDTable("MACD_EUR_USD_H1", "EUR_USD_H1");
        a.initializeMACDTable("MACD_EUR_USD_M1", "EUR_USD_M1");

        a.calcMACD("MACD_EUR_USD_H1", "EUR_USD_H1");
        a.calcMACD("MACD_EUR_USD_M1", "EUR_USD_M1");
    }

    a.analyze();

    delete a.con;
    return EXIT_SUCCESS;

    return 0;
}

Analysis::Analysis() : conn("practice") {
    driver = get_driver_instance();
    con    = driver->connect(URL, USER, PASSWORD);
    //qdb::OandaAPI ;
}

void Analysis::analyze() {
    while (true) {
        std::cout << "GET QUOTES:" << '\n';
        conn.updateAllTabs(DATABASE_NAME);

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

    // Finds the last date that has a MACD already to start from there
    sql::Statement *stmt    = con->createStatement();
    sql::ResultSet *resDate = stmt->executeQuery("SELECT date FROM " + DATABASE_NAME + "." + result + " ORDER BY date DESC LIMIT 1");
    sql::ResultSet *resEMA;

    resDate->next();

    // Grabs the starting date so it can get other data
    int MACDDate = resDate->getInt(1);

    // This looks like it is supposed to find the end date but I'm not totally
    // sure about that. It might not do anything anymore, it just gets
    // overwritten before it is used.
    //stmt    = con->createStatement();
    //resDate = stmt->executeQuery("SELECT date FROM " + DATABASE_NAME + "." + data + " ORDER BY date DESC LIMIT 1");
    //resDate->next();

    // Pulls out all the data from past where the current MACDs are. This could
    // probably be combined with what I did for resCloseAsk
    //stmt    = con->createStatement();
    resDate = stmt->executeQuery("SELECT date FROM " + DATABASE_NAME + "." + data + " WHERE date > " + std::to_string(MACDDate));

    // This prepares a statement that will insert the results into the database
    // during the loop
    sql::PreparedStatement *prep_stmt;
    prep_stmt = con->prepareStatement("INSERT INTO " + DATABASE_NAME + "." + result + "(date, EMA26, EMA12, MACD, sign, result) VALUES(?, ?, ?, ?, ?, ?)");

    // Initialze variables for use in the loop
    double EMA12    = 0.0;
    double EMA26    = 0.0;
    double sign     = 0.0;
    int    prevDate = MACDDate;

    // This code block pulls in the data from the last entry in the table
    // That way the EMAs can be calculated with much fewer queries
    resEMA = stmt->executeQuery("SELECT * FROM "+ DATABASE_NAME + "." + result + " WHERE date = " + std::to_string(MACDDate));
    resEMA->next();
    double prevEMA12 = resEMA->getDouble("EMA12");
    double prevEMA26 = resEMA->getDouble("EMA26");
    double prevSign = resEMA->getDouble("sign");

    // This code pulls in all the closeAsks so that we can work with them in
    // in memory instead of querying. That should improve speed as well.
    // Looking at it now it would be possible to combine it with resDate
    sql::ResultSet *resCloseAsk = stmt->executeQuery("SELECT closeAsk FROM " + DATABASE_NAME + "." + data + " WHERE date > " + std::to_string(MACDDate));



    // Adding time measuring code to see how long it takes
    int prevTime = time(NULL);
    double count = 0;

    // This loops through the needed dates and calculates the MACD information
    while (resDate->next()) {
        //std::cout << "~";

        // Pulls in the data from the resDate and resCloseAsk resultsets
        resCloseAsk->next();
        double closeAsk = resCloseAsk->getDouble("closeAsk");
        int date = resDate->getInt("date");

        // Calculates the two main EMAs using the stripped down method
        EMA12 = EMA(12, closeAsk, prevEMA12);
        EMA26 = EMA(26, closeAsk, prevEMA26);

        // Prepares several of the fields for the database
        prep_stmt->setInt(1, date);
        prep_stmt->setDouble(2, EMA26);
        prep_stmt->setDouble(3, EMA12);
        prep_stmt->setDouble(4, EMA12 - EMA26);

        // Calculates EMA of the difference between the averages using stripped
        // down method
        sign = EMA(9, EMA12 - EMA26, prevSign);

        // Fills in the remaining values in the prepared statement and executes
        // the prepared statement
        prep_stmt->setDouble(5, sign);
        prep_stmt->setDouble(6, (EMA12 - EMA26) - sign);
        prep_stmt->execute();

        // Stores the results of this loop so that they can be used in the next
        // loop to save on queries
        prevDate = date;
        prevEMA12 = EMA12;
        prevEMA26 = EMA26;
        prevSign = sign;

        // Prints out some performance information every hour of data
        // TODO make this section scale based on duration of data points, and
        // provide a duration estimate
        if (date % 3600 == 0) {
            count++;
            int curTime = time(NULL);
            std::cout << count << " hours took " << curTime - prevTime << ". Average: " << (curTime - prevTime) / count << '\n';
        }
    }

    // Cleanup and return success
    delete resDate;
    delete resEMA;
    delete resCloseAsk;
    delete stmt;
    return 1;
}

// This is a cut down EMA method for use in backtesting calculations and doesn't
// use any queries for more speed
double Analysis::EMA(int num, double ask, double prevEMA)
{
    // Does a quick moving average and returns the value
    double mult = 2.0 / (num + 1.0);
    return (ask - prevEMA) * mult + prevEMA;
}

// This is the full fat original EMA method for initialization purposes.
// It might be possible to use/make a cut down version, but I'm not sure
double Analysis::EMA(int num, double val, int prevDate, int newDate, std::string dataTableName, std::string resultTableName, std::string ema, std::string newDataField) {

    // Pulls in the previous EMA from the database
    sql::Statement *stmt = con->createStatement();
    sql::ResultSet *res  = stmt->executeQuery("SELECT * FROM " + DATABASE_NAME + "." + resultTableName + " WHERE date = " + std::to_string(prevDate));
    res->next();
    double prevEMA = res->getDouble(ema);

    double ask = 0.0;

    // If it isn't the 9 period sign EMA, grab the appropriate price data
    if (num != 9) {
        stmt = con->createStatement();
        res  = stmt->executeQuery("SELECT * FROM " + DATABASE_NAME + "." + dataTableName + " WHERE date = " + std::to_string(newDate));
        res->next();
        ask = res->getDouble(newDataField);
    }else {
        ask = val;
    }


    // Calculate the moving average, clean up from database connection, and
    // return
    double mult = 2.0 / (num + 1.0);
    //std::cout << prevEMA << "   " << ask << "   " << (ask - prevEMA) * mult + prevEMA << '\n';
    delete res;
    delete stmt;

    return (ask - prevEMA) * mult + prevEMA;
}


void Analysis::initializeMACDTable(std::string initialize, std::string data) {

     //  deleting old table
    sql::Statement *stmt = con->createStatement();
    // This is commented out because if there's nothing in the table to begin
    // with, this crashes it. Uncomment it if you messed up your table
    // TODO Make doing this a commandline flag so it is easy to throw everything
    // out if you need to
    //stmt->execute("TRUNCATE " + DATABASE_NAME + "." + initialize);


     //  getting the first EMA12 which is the linear average of first 12

    stmt = con->createStatement();
    sql::ResultSet *res = stmt->executeQuery("SELECT * FROM " + DATABASE_NAME + "." + data + " ORDER BY date ASC LIMIT 35");

    sql::PreparedStatement *prep_stmt;

    prep_stmt = con->prepareStatement("INSERT INTO " + DATABASE_NAME + "." + initialize + "(date, EMA12) VALUES (?, ?)");

    double asks [35];

    for (int i = 0; i < 12; i++) {
        res->next();
        asks[i] = res->getDouble("closeAsk"); // Changed this from a += to an =
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


     //  getting the next 13 EMA12 which are normal EMAs


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


     //  getting the first EMA26 (linear average of first 26) while getting next EMA12 and MACD


    prep_stmt = con->prepareStatement("INSERT INTO " + DATABASE_NAME + "." + initialize + "(date, EMA26, EMA12, MACD) VALUES (?, ?, ?, ?)");

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

    prep_stmt = con->prepareStatement("INSERT INTO " + DATABASE_NAME + "." + initialize + "(date, EMA26, EMA12, MACD, sign, result) VALUES(?, ?, ?, ?, ?, ?)");


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
