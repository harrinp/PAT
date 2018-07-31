#include "../Headers/FullBar.hpp"

/*
 *      Constructor that calls the database individually
 *      This is inefficient if a large number of FullBars are needed, use getBars instead
 */
FullBar::FullBar(int date, std::string tableName) {
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
    sqlite3_close(db);
}

/*
 *  Returns last bar of data for the given table
*/
FullBar::FullBar(std::string table){
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
        std::string table) : table(table), date(date), openBid(openBid), closeBid(closeBid), openAsk(openAsk), volume(volume), EMA26(EMA26), EMA12(EMA12), MACD(MACD), sign(sign), result(result) {
}

std::string FullBar::printableBar(){
    std::string s = std::string("date: " + std::to_string(date) );
    s += ("\nvolume: " + std::to_string(volume));
    s += ("\nopenBid: " + std::to_string(openBid));
    s += ("\ncloseBid: " + std::to_string(closeBid));
    s += ("\nopenAsk: " + std::to_string(openAsk));
    s += ("\ncloseAsk: " + std::to_string(closeAsk));
    s += ("\nEMA26: " + std::to_string(EMA26));
    s += ("\nEMA12: " + std::to_string(EMA12));
    s += ("\nMACD: " + std::to_string(MACD));
    s += ("\nsign: " + std::to_string(sign));
    s += ("\nresult: " + std::to_string(result));
    s += ("\n");
    return s;
}

/*
 *      Gets a vector of bars between start and stop dates, inclusive
 */
std::vector<FullBar> FullBar::getBarsBetween(unsigned int start, unsigned int stop, std::string tableName){
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
            0,0,0,0,0, tableName
        );
        f.closeAsk = sqlite3_column_double(data,8);
        bars.push_back(f);
    }
    rc = sqlite3_finalize(data);

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
    sqlite3_close(db);

    return bars;
}

/*
 *      Gets a vector of bars between After a start date
 */
std::vector<FullBar> FullBar::getBarsGreater(unsigned int start, std::string tableName){
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
            0,0,0,0,0, tableName
        );
        f.closeAsk = sqlite3_column_double(data,8);
        bars.push_back(f);
    }
    rc = sqlite3_finalize(data);

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
    sqlite3_close(db);

    return bars;
}

Price FullBar::convertToPrice(){
    return Price(date, closeAsk, closeBid);
}
