#include "../Headers/Decision.hpp"
#include <assert.h>

int main(int argc, char *argv[]) {
    Decider *d = NULL;

    if (argc > 1 && strcmp(argv[1], "-t") == 0) {
        d = new Tester("EUR_USD_M1", "EUR_USD_H1", .0007, .0049);
        printf("Past making the tester\n");
    }else if (argc > 1 && strcmp(argv[1], "-r") == 0)  {
        d = new Runner("EUR_USD_M1", "EUR_USD_H1", .0007, .0049);
        printf("Past making the decisioner\n");
    }else  {
        return 1;
    }
    assert(d != NULL);
    printf("decider d is not null\n");
    d->run();

    return 0;
}

Decider::Decider(bool testing, unsigned int time) : testing(testing), time(time) {
    driver = get_driver_instance();
    con    = driver->connect("tcp://127.0.0.1:3306", "root", "");
}

Decider::Pos Decider::decide(unsigned int date) {
    fillLongPrices(date);
    fillResults(date, "result");
    fillResults(date, "MACD");
    Pos p = evalMACD();

    return p;
}

Decider::Pos Decider::evalMACD() {
    if (results[0] >= 0 && results[1] < 0) {    // && MACD[1] < 0){
        return LONGING;
    }else if (results[0] < 0 && results[1] >= 0) {
        return SHORTING;
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
        return LONGING;
    }else {
        return SHORTING;
    }
}

void Decider::fillResults(int date, std::string col) {
    sql::Statement *stmt = con->createStatement();
    sql::ResultSet *res  = stmt->executeQuery("SELECT " + col + " FROM " + DATABASE_NAME + ".MACD_" + longerTable + " WHERE date = " + std::to_string(date));

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
    sql::ResultSet *res  = stmt->executeQuery(
            "SELECT * FROM " + DATABASE_NAME + ".MACD_" + longerTable + " WHERE date = " + std::to_string(date));

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

Decider::Pos Decider::decide(FullBar * f) {
    fillLongPrices(f);
    fillResults(f, "result");
    fillResults(f, "MACD");
    Pos p = evalMACD();

    return p;
}

void Decider::fillResults(FullBar * f, std::string col) {
    if (col == "MACD") {
        MACD[0] = MACD[1];
        MACD[1] = f->MACD;//res->getDouble(col);
    }else {
        results[0] = results[1];
        results[1] = f->result;//res->getDouble(col);
    }
}

void Decider::fillLongPrices(FullBar * f) {

    for (int i = 0; i < 9; i++) {
        EMA12[i] = EMA12[i + 1];
    }
    EMA12[9] = f->EMA12;

    for (int i = 0; i < 9; i++) {
        EMA26[i] = EMA26[i + 1];
    }
    EMA26[9] = f->EMA26;

}
