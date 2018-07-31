#include "Headers/Decision.hpp"

Tester::Tester(std::string table, std::string longerTable, double takeProfit, double stopLoss, std::vector <FullBar> *data, std::vector <FullBar> *dataLong) :
    Decider(true, 0),
    minutes(data),
    hours(dataLong),
    back(10000.0, table, 50.0) {
    this->stopLoss    = stopLoss;
    this->takeProfit  = takeProfit;
    this->table       = table;
    this->longerTable = longerTable;
    verbose           = false;
}

Tester::Tester(std::string table, std::string longerTable, double takeProfit, double stopLoss) :
    Decider(true, 0),
    back(10000.0, table, 50.0) {
    this->stopLoss    = stopLoss;
    this->takeProfit  = takeProfit;
    this->table       = table;
    this->longerTable = longerTable;
    verbose           = false;
}

void Tester::closeProfitableOrNotTrades(Price p) {
    for (int i = 0; i < (int)back.trades.size(); i++) {
        if (back.trades[i].calcProfit(p) < -1 * stopLoss * back.trades[i].units * p.ask ||
            back.trades[i].calcProfit(p) > takeProfit * back.trades[i].units * p.ask
            ) {
            back.closeTrade(p, i);
        }
    }
}

void Tester::runMany() {
    Price p   = Price(0, 0, 0);
    Pos   pos = NOTHING;

    int date        = 0;
    int counter     = 0;
    int counterLong = 0;
    //std::cout << "/* message 2 */" << '\n';
    while (counter < minutes->size() - 5 && counterLong < hours->size() - 5) {
        //std::cout << counter << " - " << counterLong << '\n';
        //verbose = false;
        if (counter % 1440 == 0) {
            //std::cout << "day " << counter / 1440 << " --- Profit: " << back.USD << "\n";
        }
        date = minutes->at(counter).date;
        p    = Price(date, minutes->at(counter).closeAsk, minutes->at(counter).closeBid);

        closeProfitableOrNotTrades(p);

        if (date >= hours->at(counterLong).date) {
            //std::cout << "HERE" << '\n';
            pos = decide(&hours->at(counterLong));
            switch (pos) {
            case SHORTING:
                //back.closeAll(p);
                back.makeTrade(SHORT, (int)(0.01 * back.USD * 50), p);
                break;

            case LONGING:
                //back.closeAll(p);
                back.makeTrade(LONG, (int)(0.01 * back.USD * 50), p);
                break;

            case NOTHING: break;
            }
            counterLong++;
        }
        //std::cout << "/* message 3 */ " << counter << "  " << date << '\n';

        counter++;
    }
    back.closeAll(Price(date, minutes->at(counter).closeAsk, minutes->at(counter).closeBid));
    if (verbose) {
        back.reportTrades();
    }
}

void Tester::run() {
    Price p   = Price(0, 0, 0);
    Pos   pos = NOTHING;


    std::vector <FullBar> data = FullBar::getBarsBetween(1230804061, 1532983671, table);

    std::vector <FullBar> dataLong = FullBar::getBarsBetween(1230804061, 1532983671, longerTable);

    int date        = 0;
    int counter     = 0;
    int counterLong = 0;
    //std::cout << "/* message 2 */" << '\n';
    while (counter < data.size() - 5 && counterLong < dataLong.size() - 5) {
        //std::cout << counter << " - " << counterLong << '\n';
        //verbose = false;
        if (counter % 1440 == 0) {
            //std::cout << "day " << counter / 1440 << " --- Profit: " << back.USD << "\n";
        }
        date = data.at(counter).date;
        p    = data.at(counter).convertToPrice();

        closeProfitableOrNotTrades(p);

        if (date >= dataLong.at(counterLong).date) {
            //std::cout << "HERE" << '\n';
            pos = decide(&dataLong.at(counterLong));
            switch (pos) {
            case SHORTING:
                //back.closeAll(p);
                back.makeTrade(SHORT, (int)(0.01 * back.USD * 50), p);
                break;

            case LONGING:
                //back.closeAll(p);
                back.makeTrade(LONG, (int)(0.01 * back.USD * 50), p);
                break;

            case NOTHING: break;
            }
            counterLong++;
        }
        //std::cout << "/* message 3 */ " << counter << "  " << date << '\n';

        counter++;
    }
    back.closeAll(data.at(counter).convertToPrice());
    if (verbose) {
        back.reportTrades();
    }
}

double Tester::getTotal() {
    return back.USD;
}
