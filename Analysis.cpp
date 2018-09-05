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

        a.calcMACD("EUR_USD_H1");
        a.calcMACD("EUR_USD_M1");
    }

    a.analyze();

    delete a.con;
    return EXIT_SUCCESS;

    return 0;
}

Analysis::Analysis() : conn("practice")
{
    // creating a connection
    int rc;
    rc = sqlite3_open(DATABASE_NAME.c_str(), &db);
    if (rc)
    {
      fprintf(stderr, "Can't open database %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
    }
    // con is the pure mysql connection
    //qdb::OandaAPI ;
}

void Analysis::analyze() {
    while (true) {
        std::cout << "GET QUOTES:" << '\n';
        conn.updateAllTabs(DATABASE_NAME);

        std::cout << "ANALYZE H AND M" << '\n';
        calcMACD("EUR_USD_M1");
        std::cout << "DONE M" << '\n';
        calcMACD("EUR_USD_H1");
        std::cout << "DONE H" << '\n';
        std::cout << "Sleeping..." << '\n';
        usleep(30000000);   //30 Seconds
    }
}

int Analysis::calcMACD(std::string dataTable)
{
    std::vector<FullBar> bars = FullBar::getAnalysisBars(dataTable);

    double EMA12    = 0.0;
    double EMA26    = 0.0;
    double sign     = 0.0;
    int    prevDate = bars[0].date; 
    double prevEMA12 = bars[0].EMA12;
    double prevEMA26 = bars[0].EMA26;
    double prevSign = bars[0].sign;
    
    bars.erase(bars.begin()); // Gets rid of the already analyzed Bar

    // This loops through the needed dates and calculates the MACD information
    for(int i = 0 ; i < bars.size() ; i++)
    {
        // Pulls in the data from the resDate and resCloseAsk resultsets
        double closeAsk = bars[i].closeAsk;
        int date = bars[i].date;

        // Calculates the two main EMAs using the stripped down method
        EMA12 = EMA(12, closeAsk, prevEMA12);
        EMA26 = EMA(26, closeAsk, prevEMA26);

        // Puts values into the bar
        bars[i].EMA12 = EMA12;
        bars[i].EMA26 = EMA26;
        bars[i].MACD = EMA12 - EMA26;
       
        // Calculates EMA of the difference between the averages using stripped
        // down method
        sign = EMA(9, EMA12 - EMA26, prevSign);

        // Fills in the remaining values in the bar
        bars[i].sign = sign;
        bars[i].result = (EMA12 - EMA26) - sign;
        
        // Stores the results of this loop so that they can be used in the next
        // loop to save on queries
        prevDate = date;
        prevEMA12 = EMA12;
        prevEMA26 = EMA26;
        prevSign = sign;
        
    }

    // TODO add the thing to write the bars back to the table
    FullBar::putAnalysisBars(bars, dataTable); 
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
}
