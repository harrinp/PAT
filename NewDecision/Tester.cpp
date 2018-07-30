#include "../Headers/Decision.hpp"

Tester::Tester(std::string table, std::string longerTable, double takeProfit, double stopLoss) :
    Decider(true, 0),
    back(6000.0, table, 50.0) {
    this->stopLoss    = stopLoss;
    this->takeProfit  = takeProfit;
    this->table       = table;
    this->longerTable = longerTable;
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

void Tester::run() {
    Price p   = Price(0, 0, 0);
    Pos   pos = NOTHING;


    std::vector <FullBar> data = FullBar::getBarsGreater(EARLIEST_TEST_DATE, "EUR_USD_M1");
    std::vector <FullBar> dataLong = FullBar::getBarsGreater(EARLIEST_TEST_DATE, "EUR_USD_H1");

    int date    = 0;
    int counter = 0;
    int counterLong = 0;
    while (counter < data.size() && counter < 10 * 365 * 1440) {
        if (counter % 1440 == 0) {
            //std::cout << "day " << counter / 1440 << " --- Profit: " << back.USD << "\n";
        }
        date = data[counter].date;
        p    = Price(date, data[counter].closeAsk, data[counter].closeBid);

        closeProfitableOrNotTrades(p);

        if (counter % 60 == 0){
            pos = decide(&dataLong[counterLong]);
            switch (pos) {
            case SHORTING:
                back.closeAll(p);
                back.makeTrade(SHORT, (int)(0.01 * back.USD * 50), p);
                break;

            case LONGING:
                back.closeAll(p);
                back.makeTrade(LONG, (int)(0.01 * back.USD * 50), p);
                break;

            case NOTHING: break;
            }
            counterLong++;
        }


        counter++;
    }
    back.closeAll(Price(date, data[counter].closeAsk, data[counter].closeBid));
    back.reportTrades();
}
