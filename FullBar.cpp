#include "Headers/FullBar.hpp"

/*
 *      All constructors and functions that create vectors have a final bool parameter
 *      This bool indicates whether there is MACD data for this bar
 *      Set to false to skip queries to the MACD table
 */

/*
 *      Constructor that calls the database individually
 *      This is inefficient if a large number of FullBars are needed, use getBars instead
 *      gets bar at the date given by the date parameter
 */
FullBar::FullBar(int date, std::string tableName, bool hasMACD) {
    sqlite3 *db;
    sqlite3_stmt * data;
    int rc; // Return code

    rc = sqlite3_open(DATABASE_NAME.c_str(), &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_prepare_v2(db, ("SELECT * FROM " + tableName + " WHERE date = " + std::to_string(date) + ";").c_str(), -1 , &data, NULL);

    if( rc ) {
        fprintf(stderr, "Can't prepare statement: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_step(data);

    this->date = (sqlite3_column_double(data, 0));
    this->openBid = sqlite3_column_double(data,1);
    this->openAsk = sqlite3_column_double(data,2);
    this->closeBid = sqlite3_column_double(data,7);
    this->closeAsk = sqlite3_column_double(data,8);
    this->volume = sqlite3_column_int(data,9);

    rc = sqlite3_finalize(data);
    if( rc ) {
        fprintf(stderr, "Failed to finalize: %s\n", sqlite3_errmsg(db));
    }
    if (hasMACD){
        rc = sqlite3_prepare_v2(db, ("SELECT * FROM MACD_" + tableName + " WHERE date = " + std::to_string(date) + ";").c_str(), -1 , &data, NULL);
        if( rc ) {
            fprintf(stderr, "Can't prepare statement: %s\n", sqlite3_errmsg(db));
        }

        rc = sqlite3_step(data);

        this->EMA26 = sqlite3_column_double(data, 1);
        this->EMA12 = sqlite3_column_double(data, 2);
        this->MACD = sqlite3_column_double(data, 3);
        this->sign = sqlite3_column_double(data, 4);
        this->result = sqlite3_column_double(data, 5);

        rc = sqlite3_finalize(data);
    }

    sqlite3_close(db);
}

/*
 *  Returns last bar of data for the given table
*/
FullBar::FullBar(std::string table, bool hasMACD){
    sqlite3 *db;
    sqlite3_stmt * data;
    int rc; // Return code

    rc = sqlite3_open(DATABASE_NAME.c_str(), &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_prepare_v2(db, ("SELECT * FROM " + table + " ORDER BY DATE DESC LIMIT 1;").c_str(), -1 , &data, NULL);

    if( rc ) {
        fprintf(stderr, "Can't prepare statement: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_step(data);

    this->date = (sqlite3_column_double(data, 0));
    this->openBid = sqlite3_column_double(data,1);
    this->openAsk = sqlite3_column_double(data,2);
    this->closeBid = sqlite3_column_double(data,7);
    this->closeAsk = sqlite3_column_double(data,8);
    this->volume = sqlite3_column_int(data,9);

    rc = sqlite3_finalize(data);
    if( rc ) {
        fprintf(stderr, "Failed to finalize: %s\n", sqlite3_errmsg(db));
    }
    if (hasMACD){
        rc = sqlite3_prepare_v2(db, ("SELECT * FROM MACD_" + table + " ORDER BY DATE DESC LIMIT 1;").c_str(), -1 , &data, NULL);
        if( rc ) {
            fprintf(stderr, "Can't prepare statement: %s\n", sqlite3_errmsg(db));
        }

        rc = sqlite3_step(data);

        this->EMA26 = sqlite3_column_double(data, 1);
        this->EMA12 = sqlite3_column_double(data, 2);
        this->MACD = sqlite3_column_double(data, 3);
        this->sign = sqlite3_column_double(data, 4);
        this->result = sqlite3_column_double(data, 5);

        rc = sqlite3_finalize(data);
    }

    sqlite3_close(db);
}

/*
 *      Constructor for raw data
 *      No sql calls are made
 */
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
        double result,
        std::string table, bool hasMACD) :  table(table), hasMACD(hasMACD), date(date), openBid(openBid), closeBid(closeBid), openAsk(openAsk), volume(volume), EMA26(EMA26), EMA12(EMA12), MACD(MACD), sign(sign), result(result) {
}

std::string FullBar::printableBar(){
    std::string s = std::string("date: " + std::to_string(date) );
    s += ("\nvolume: " + std::to_string(volume));
    s += ("\nopenBid: " + std::to_string(openBid));
    s += ("\ncloseBid: " + std::to_string(closeBid));
    s += ("\nopenAsk: " + std::to_string(openAsk));
    s += ("\ncloseAsk: " + std::to_string(closeAsk));
    if (hasMACD){
        s += ("\nEMA26: " + std::to_string(EMA26));
        s += ("\nEMA12: " + std::to_string(EMA12));
        s += ("\nMACD: " + std::to_string(MACD));
        s += ("\nsign: " + std::to_string(sign));
        s += ("\nresult: " + std::to_string(result));
    }
    else {
        s += "\nNo MACD data";
    }

    s += ("\n");
    return s;
}

/*
 *      Gets a vector of bars between start and stop dates, inclusive
 */
std::vector<FullBar> FullBar::getBarsBetween(unsigned int start, unsigned int stop, std::string tableName, bool hasMACD){
    sqlite3 *db;
    sqlite3_stmt * data;
    int rc;     // return code

    std::vector<FullBar> bars;

    rc = sqlite3_open(DATABASE_NAME.c_str(), &db);
    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_prepare_v2(db, ("SELECT * FROM " + tableName + " WHERE date BETWEEN " + std::to_string(start) + " AND "+ std::to_string(stop) + " ORDER BY date ASC;").c_str(), -1 , &data, NULL);
    if( rc ) {
        fprintf(stderr, "Can't prepare statement: %s\n", sqlite3_errmsg(db));
    }

    while (sqlite3_step(data) != SQLITE_DONE) {
        FullBar f = FullBar(
            sqlite3_column_int(data, 0),
            sqlite3_column_int(data, 9),
            sqlite3_column_double(data,1),
            sqlite3_column_double(data,7),
            sqlite3_column_double(data,2),
            sqlite3_column_double(data,8),
            0,0,0,0,0, tableName, true
        );
        f.closeAsk = sqlite3_column_double(data,8);
        bars.push_back(f);
    }
    rc = sqlite3_finalize(data);

    if (hasMACD){
        rc = sqlite3_prepare_v2(db, ("SELECT * FROM MACD_" + tableName + " WHERE date BETWEEN " + std::to_string(start) + " AND "+ std::to_string(stop) + " ORDER BY date ASC;").c_str(), -1 , &data, NULL);
        if( rc ) {
            fprintf(stderr, "Can't prepare statement: %s\n", sqlite3_errmsg(db));
        }

        int count = 0;
        while (sqlite3_step(data) != SQLITE_DONE) {
            bars[count].EMA26 = sqlite3_column_double(data, 1);
            bars[count].EMA12 = sqlite3_column_double(data, 2);
            bars[count].MACD = sqlite3_column_double(data, 3);
            bars[count].sign = sqlite3_column_double(data, 4);
            bars[count].result = sqlite3_column_double(data, 5);
            count++;
        }

        rc = sqlite3_finalize(data);
    }

    sqlite3_close(db);

    return bars;
}

/*
 *      Gets a vector of bars after a start date
 */
std::vector<FullBar> FullBar::getBarsGreater(unsigned int start, std::string tableName, bool hasMACD){
    sqlite3 *db;
    sqlite3_stmt * data;
    int rc;     // return code

    std::vector<FullBar> bars;

    rc = sqlite3_open(DATABASE_NAME.c_str(), &db);
    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_prepare_v2(db, ("SELECT * FROM " + tableName + " WHERE date > " + std::to_string(start) + " ORDER BY date ASC;").c_str(), -1 , &data, NULL);
    if( rc ) {
        fprintf(stderr, "Can't prepare statement: %s\n", sqlite3_errmsg(db));
    }

    while (sqlite3_step(data) != SQLITE_DONE) {
        FullBar f = FullBar(
            sqlite3_column_int(data, 0),
            sqlite3_column_int(data, 9),
            sqlite3_column_double(data,1),
            sqlite3_column_double(data,7),
            sqlite3_column_double(data,2),
            sqlite3_column_double(data,8),
            0,0,0,0,0, tableName, hasMACD
        );
        f.closeAsk = sqlite3_column_double(data,8);
        bars.push_back(f);
    }
    rc = sqlite3_finalize(data);

    if (hasMACD){   // If the data has MACD data query it
        rc = sqlite3_prepare_v2(db, ("SELECT * FROM MACD_" + tableName + " WHERE date > " + std::to_string(start) + " ORDER BY date ASC;").c_str(), -1 , &data, NULL);
        if( rc ) {
            fprintf(stderr, "Can't prepare statement: %s\n", sqlite3_errmsg(db));
        }

        int count = 0;
        while (sqlite3_step(data) != SQLITE_DONE) {
            bars[count].EMA26 = sqlite3_column_double(data, 1);
            bars[count].EMA12 = sqlite3_column_double(data, 2);
            bars[count].MACD = sqlite3_column_double(data, 3);
            bars[count].sign = sqlite3_column_double(data, 4);
            bars[count].result = sqlite3_column_double(data, 5);
            count++;
        }

        rc = sqlite3_finalize(data);
    }

    sqlite3_close(db);

    return bars;
}
/*
 * Gets bars that haven't been analyzed yet (and one that has)
 */
std::vector<FullBar> FullBar::getAnalysisBars(std::string tableName) {
    sqlite3 *db;
    sqlite3_stmt *data;
    int rc; // return code

    rc = sqlite3_open(DATABASE_NAME.c_str(), &db);
    if(rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_prepare_v2(db, ("SELECT date FROM MACD_" + tableName + " ORDER BY date DESC LIMIT 2").c_str(), -1, &data, NULL);
    if(rc) {
        fprintf(stderr, "Can't prepare statement: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_step(data);
    sqlite3_step(data);

    int startDate = sqlite3_column_int(data, 0);

    sqlite3_finalize(data);
    sqlite3_close(db);

    std::vector<FullBar> bars = getBarsGreater(startDate, tableName, true);
    return bars;

}

// TODO add a writeBars method
int FullBar::putAnalysisBars(std::vector<FullBar> barsToPut, std::string tableName) {
    sqlite3 *db;
    sqlite3_stmt *insert;
    int rc; // return code

    rc = sqlite3_open(DATABASE_NAME.c_str(), &db);
    if(rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_prepare_v2(db, ("INSERT INTO MACD_" + tableName + " (date, EMA26, EMA12, MACD, sign, result) VALUES (?,?,?,?,?,?)").c_str(), -1, &insert, NULL);
    if(rc) {
        fprintf(stderr, "Can't prepare statement: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, 0 , NULL);
    if(rc) {
        fprintf(stderr, "Can't prepare statement: %s\n", sqlite3_errmsg(db));
    }
    for(auto bar : barsToPut) {
        sqlite3_bind_int(insert, 1, bar.date);
        sqlite3_bind_double(insert, 2, bar.EMA26);
        sqlite3_bind_double(insert, 3, bar.EMA12);
        sqlite3_bind_double(insert, 4, bar.MACD);
        sqlite3_bind_double(insert, 5, bar.sign);
        sqlite3_bind_double(insert, 6, bar.result);
        sqlite3_step(insert);
        sqlite3_reset(insert);
        sqlite3_clear_bindings(insert);
    }
    
    sqlite3_exec(db, "END TRANSACTION", NULL, 0, NULL);
    sqlite3_finalize(insert);
    return 1;
}



Price FullBar::convertToPrice(){
    return Price(date, closeAsk, closeBid);
}
