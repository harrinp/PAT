#include "Headers/Decision.hpp"

Decider::Decider(bool testing, unsigned int time) : testing(testing), time(time) {
}

Decider::Pos Decider::decide(unsigned int date){

    Pos p = evalMACD();

    return p;
}

Decider::Pos Decider::evalMACD() {
    if (results[0] >= 0 && results[1] < 0) {    // && MACD[1] < 0){
        return LONG;
    }else if (results[0] < 0 && results[1] >= 0) {
        return SHORT;
    }else {
        return NOTHING;
    }
}

Decider::Pos Decider::evalEMA() {
    //bool alreadySignaled = false;
    double diff = EMA12[9] - EMA26[9];

    if (((EMA12[8] - EMA26[8] < 0 && diff < 0) || (EMA12[8] - EMA26[8] > 0 && diff > 0))) {
        return NOTHING;
    }else if (EMA12[9] > EMA26[9]) {
        return LONG;
    }else {
        return SHORT;
    }
}

void Decider::fillResults(int date, std::string col) {
    sql::Statement *stmt = con->createStatement();
    sql::ResultSet *res  = stmt->executeQuery("SELECT " + col + " FROM quotesdb.MACD_" + longerTable + " WHERE date = " + std::to_string(date));

    res->next();
    if (col == "MACD") {
        MACD[0] = MACD[1];
        MACD[1] = res->getDouble(col);
    }else {
        results[0] = results[1];
        results[1] = res->getDouble(col);
    }
    delete res;
    delete stmt;
}

void Decider::fillLongPrices(int date) {
    sql::Statement *stmt = con->createStatement();
    sql::ResultSet *res  = stmt->executeQuery("SELECT * FROM quotesdb.MACD_" + longerTable + " WHERE date = " + std::to_string(date));

    res->next();
    for (int i = 0; i < 9; i++) {
        EMA12[i] = EMA12[i + 1];
    }
    EMA12[9] = res->getDouble("EMA12");

    for (int i = 0; i < 9; i++) {
        EMA26[i] = EMA26[i + 1];
    }
    EMA26[9] = res->getDouble("EMA26");
    delete res;
    delete stmt;
}


Tester::Tester(std::string table, std::string longerTable, double takeProfit, double stopLoss) :
    Decider(true, 0),
    back(6000.0, table, 50.0) {
    this->stopLoss    = stopLoss;
    this->takeProfit  = takeProfit;
    this->table       = table;
    this->longerTable = longerTable;
}

Decisioner::Decisioner(std::string table, std::string longerTable, double takeProfit, double stopLoss) :
    Decider(true, 0),
    exec() {
    this->stopLoss    = stopLoss;
    this->takeProfit  = takeProfit;
    this->table       = table;
    this->longerTable = longerTable;
}
