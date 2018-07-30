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

Analysis::Analysis() : conn("practice")
{
    // creating a connection
    char *zErrorMessage = 0;
    int rc;
    rc = sqlite3_open(DATABASE_NAME.c_str(), &db);
    if (rc)
    {
      fprintf(stderr, "Can't open database %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
    }

    driver = get_driver_instance();
    con    = driver->connect(URL, USER, PASSWORD);      // con is the pure mysql connection
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

int Analysis::calcMACD(std::string resultTable, std::string dataTable)
{

    int rc;
    sqlite3_stmt *resDate;
    sqlite3_stmt *resEMA;
    sqlite3_stmt *resCloseAsk;
    sqlite3_stmt *insert;

    rc = sqlite3_prepare_v2(db, ("SELECT date FROM " + resultTable + " ORDER BY date DESC LIMIT 1").c_str(), -1, &resDate, NULL);
    if (rc)
    {
      fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_step(resDate);
    int MACDDate = sqlite3_column_int(resDate, NULL);

    // QUESTION Should this have an enforced order?
    rc = sqlite3_prepare_v2(db, ("SELECT date FROM " + dataTable + " WHERE date > " + std::to_string(MACDDate)).c_str(), -1, &resDate, NULL);
    if (rc)
    {
      fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(db));
    }
    rc = sqlite3_prepare_v2(db, ("INSERT INTO " + resultTable + " (date, EMA26, EMA12, MACD, sign, result) VALUES(?, ?, ?, ?, ?, ?)").c_str(), -1, &insert, NULL);
    if (rc)
    {
      fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(db));
    }

    double EMA12    = 0.0;
    double EMA26    = 0.0;
    double sign     = 0.0;
    int    prevDate = MACDDate;

    rc = sqlite3_prepare_v2(db, ("SELECT * FROM " + resultTable + " WHERE date = " + std::to_string(MACDDate)).c_str(), -1, &resEMA, NULL);
    if (rc)
    {
      fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_step(resEMA);
    double prevEMA12 = sqlite3_column_double(resEMA, 2);
    double prevEMA26 = sqlite3_column_double(resEMA, 1);
    double prevSign = sqlite3_column_double(resEMA, 4);

    // QUESTION Should this have an enforced order?
    rc = sqlite3_prepare_v2(db, ("SELECT closeAsk FROM " + dataTable + " WHERE date > " + std::to_string(MACDDate)).c_str(), -1, &resCloseAsk, NULL);
    if (rc)
    {
      fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(db));
    }

    // Adding time measuring code to see how long it takes
    int prevTime = time(NULL);
    int count = 0;

    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, 0, NULL);
    // This loops through the needed dates and calculates the MACD information
    while (sqlite3_step(resDate) == 100)
    {
        //std::cout << "~";

        // Pulls in the data from the resDate and resCloseAsk resultsets
        sqlite3_step(resCloseAsk);
        double closeAsk = sqlite3_column_double(resCloseAsk, 0);
        int date = sqlite3_column_int(resDate, 0);

        // Calculates the two main EMAs using the stripped down method
        EMA12 = EMA(12, closeAsk, prevEMA12);
        EMA26 = EMA(26, closeAsk, prevEMA26);

        // Prepares several of the fields for the database
        sqlite3_bind_int(insert, 1, date);
        sqlite3_bind_double(insert, 2, EMA26);
        sqlite3_bind_double(insert, 3, EMA12);
        sqlite3_bind_double(insert, 4, EMA12 - EMA26);

        // Calculates EMA of the difference between the averages using stripped
        // down method
        sign = EMA(9, EMA12 - EMA26, prevSign);

        // Fills in the remaining values in the prepared statement and executes
        // the prepared statement
        sqlite3_bind_double(insert, 5, sign);
        sqlite3_bind_double(insert, 6, (EMA12 - EMA26) - sign);
        sqlite3_step(insert);
        sqlite3_reset(insert);
        sqlite3_clear_bindings(insert);

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
            std::cout << dataTable + ": " << count << " hours took " << curTime - prevTime << ". Average: " << (curTime - prevTime) / (double)count << '\n';
        }
    }
    sqlite3_exec(db, "END TRANSACTION", NULL, 0, NULL);
    sqlite3_finalize(resDate);
    sqlite3_finalize(insert);
    sqlite3_finalize(resEMA);
    sqlite3_finalize(resCloseAsk);
    printf("made it\n");
    return 1;
    /*
    //    std::cout << "RUNNING" << '\n';

    // Finds the last date that has a MACD already to start from there
    sql::Statement *stmt    = con->createStatement();
    sql::ResultSet *resDate = stmt->executeQuery("SELECT date FROM " + DATABASE_NAME + "." + resultTable + " ORDER BY date DESC LIMIT 1");
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
    resDate = stmt->executeQuery("SELECT date FROM " + DATABASE_NAME + "." + dataTable + " WHERE date > " + std::to_string(MACDDate));

    // This prepares a statement that will insert the results into the database
    // during the loop
    sql::PreparedStatement *prep_stmt;
    prep_stmt = con->prepareStatement("INSERT INTO " + DATABASE_NAME + "." + resultTable + "(date, EMA26, EMA12, MACD, sign, result) VALUES(?, ?, ?, ?, ?, ?)");

    // Initialze variables for use in the loop
    double EMA12    = 0.0;
    double EMA26    = 0.0;
    double sign     = 0.0;
    int    prevDate = MACDDate;

    // This code block pulls in the data from the last entry in the table
    // That way the EMAs can be calculated with much fewer queries
    resEMA = stmt->executeQuery("SELECT * FROM "+ DATABASE_NAME + "." + resultTable + " WHERE date = " + std::to_string(MACDDate));
    resEMA->next();
    double prevEMA12 = resEMA->getDouble("EMA12");
    double prevEMA26 = resEMA->getDouble("EMA26");
    double prevSign = resEMA->getDouble("sign");

    // This code pulls in all the closeAsks so that we can work with them in
    // in memory instead of querying. That should improve speed as well.
    // Looking at it now it would be possible to combine it with resDate
    sql::ResultSet *resCloseAsk = stmt->executeQuery("SELECT closeAsk FROM " + DATABASE_NAME + "." + dataTable + " WHERE date > " + std::to_string(MACDDate));



    // Adding time measuring code to see how long it takes
    int prevTime = time(NULL);
    int count = 0;

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
            std::cout << dataTable + ": " << count << " hours took " << curTime - prevTime << ". Average: " << (curTime - prevTime) / (double)count << '\n';
        }
    }

    // Cleanup and return success
    delete resDate;
    delete resEMA;
    delete resCloseAsk;
    delete stmt;
    return 1;
    */
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
// NOTE This is no longer used, and will probably be deleted.
double Analysis::EMA(int num, double val, int prevDate, int newDate, std::string dataTableName, std::string resultTableName, std::string ema, std::string newDataField) {

    /*
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
    */
}


void Analysis::initializeMACDTable(std::string initialize, std::string data) {

     //  deleting old table

    int rc;
    rc = sqlite3_exec(db, ("DROP TABLE IF EXISTS " + initialize).c_str(), NULL, 0, NULL);
    if (rc)
    {
      fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(db));
    }
    rc = sqlite3_exec(db, ("CREATE TABLE " + initialize + " (date integer unsigned NOT NULL, EMA26 decimal(8,5) DEFAULT NULL, "
                           "EMA12 decimal(8,5) DEFAULT NULL, MACD decimal(8,5) DEFAULT NULL, sign decimal(8,5) DEFAULT NULL, "
                           "result decimal(8,5) DEFAULT NULL)").c_str(), NULL, 0, NULL);
    if (rc)
    {
      fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(db));
    }
    // This is commented out because if there's nothing in the table to begin
    // with, this crashes it. Uncomment it if you messed up your table
    // TODO Make doing this a commandline flag so it is easy to throw everything
    // out if you need to
    //stmt->execute("TRUNCATE " + DATABASE_NAME + "." + initialize);


     //  getting the first EMA12 which is the linear average of first 12
    sqlite3_stmt *res;
    rc = sqlite3_prepare_v2(db, ("SELECT * FROM " + data + " ORDER BY date ASC LIMIT 35").c_str(), -1, &res, NULL);
    if (rc)
    {
      fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_stmt *insert;
    rc = sqlite3_prepare_v2(db, ("INSERT INTO " + initialize + " (date, EMA12) VALUES (?, ?)").c_str(), -1, &insert, NULL);
    if (rc)
    {
      fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(db));
    }

    double asks [35];

    for(int i = 0 ; i < 12 ; i++)
    {
      sqlite3_step(res);
      asks[i] = sqlite3_column_double(res, 8); // Gets the closeAsk
    }

    double ave   = 0.0;
    double count = 0.0;

    for (int i = 11; i >= 0; i--)
    {
        ave   += asks[i] * (i + 1);
        count += i + 1;
    }
    ave /= count;

    int prevDate = sqlite3_column_int(res, 0); // Gets the date

    sqlite3_bind_int(insert, 1, prevDate);
    sqlite3_bind_double(insert, 2, ave);
    sqlite3_step(insert);
    sqlite3_reset(insert);
    sqlite3_clear_bindings(insert);

    //  getting the next 13 EMA12 which are normal EMAs

    double EMA12 = 0;
    double prevEMA12 = ave;

    for (int i = 0; i < 13; i++)
    {
        sqlite3_step(res);
        asks[12 + i] = sqlite3_column_double(res, 8); // Gets the closeAsk


        EMA12    = EMA(12, asks[12+i], prevEMA12);
        prevEMA12 = EMA12;
        prevDate = sqlite3_column_int(res, 0); // Gets the date

        //std::cout << "EMA12:" << EMA12 << '\n';

        sqlite3_bind_int(insert, 1, prevDate);
        sqlite3_bind_double(insert, 2, EMA12);
        sqlite3_step(insert);
        sqlite3_reset(insert);
        sqlite3_clear_bindings(insert);
    }

    //  getting the first EMA26 (linear average of first 26) while getting next EMA12 and MACD
    rc = sqlite3_prepare_v2(db, ("INSERT INTO " + initialize + " (date, EMA26, EMA12, MACD) VALUES (?, ?, ?, ?)").c_str(), -1, &insert, NULL);
    if (rc)
    {
      fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_step(res);

    double MACDs [9];

    ave   = 0.0;
    count = 0.0;
    for (int i = 24; i >= 0; i--)
    {
        ave   += asks[i] * (i + 1);
        count += i + 1;
    }
    ave /= count;
    EMA12 = EMA(12, sqlite3_column_double(res, 8), prevEMA12);
    prevEMA12 = EMA12;

    sqlite3_bind_int(insert, 1, sqlite3_column_int(res, 0));
    sqlite3_bind_double(insert, 2, ave);
    sqlite3_bind_double(insert, 3, EMA12);
    sqlite3_bind_double(insert, 4, EMA12 - ave);
    sqlite3_step(insert);
    sqlite3_reset(insert);
    sqlite3_clear_bindings(insert);
    MACDs[0] = EMA12 - ave;

    double EMA26 = 0.0;
    double prevEMA26 = ave;
    double closeAsk = 0.0;

    for (int i = 0; i < 7; i++)
    {
        sqlite3_step(res);
        closeAsk = sqlite3_column_double(res, 8);
        EMA12 = EMA(12, closeAsk, prevEMA12);
        prevEMA12 = EMA12;
        EMA26 = EMA(26, closeAsk, prevEMA26);
        prevEMA26 = EMA26;
        sqlite3_bind_int(insert, 1, sqlite3_column_int(res, 0));
        sqlite3_bind_double(insert, 2, EMA26);
        sqlite3_bind_double(insert, 3, EMA12);
        sqlite3_bind_double(insert, 4, EMA12 - EMA26);
        sqlite3_step(insert);
        sqlite3_reset(insert);
        sqlite3_clear_bindings(insert);
        MACDs[1 + i] = EMA12 - EMA26;
    }

    sqlite3_step(res);
    closeAsk = sqlite3_column_double(res, 8);
    EMA12 = EMA(12, closeAsk, prevEMA12);
    prevEMA12 = EMA12;
    EMA26 = EMA(26, closeAsk, prevEMA26);
    prevEMA26 = EMA26;

    MACDs[8] = EMA12 - EMA26;

    rc = sqlite3_prepare_v2(db, ("INSERT INTO " + initialize + " (date, EMA26, EMA12, MACD, sign, result) VALUES(?, ?, ?, ?, ?, ?)").c_str(), -1, &insert, NULL);
    if (rc)
    {
      fprintf(stderr, "Database error: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_bind_int(insert, 1, sqlite3_column_int(res, 0));
    sqlite3_bind_double(insert, 2, EMA26);
    sqlite3_bind_double(insert, 3, EMA12);
    sqlite3_bind_double(insert, 4, EMA12 - EMA26);

    double sig = 0.0;
    count = 0.0;
    for (int i = 8; i >= 0; i--)
    {
        sig   += MACDs[i] * (i + 1);
        count += i + 1;
    }
    sig /= count;

    sqlite3_bind_double(insert, 5, sig);
    sqlite3_bind_double(insert, 6, MACDs[8] - sig);
    sqlite3_step(insert);
    sqlite3_reset(insert);
    sqlite3_clear_bindings(insert);

    sqlite3_finalize(res);
    sqlite3_finalize(insert);
    printf("made it\n");
    /*
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
    */
}
