#include "../Headers/Decision.hpp"

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "-m") == 0){
        double bestI = 0;
        double bestJ = 0;
        double bestProfit = 0;

        std::vector <FullBar> data = FullBar::getBarsBetween(EARLIEST_TEST_DATE, 1293876061, "EUR_USD_M1");
        std::vector <FullBar> dataLong = FullBar::getBarsBetween(EARLIEST_TEST_DATE, 1293876061, "EUR_USD_H1");
        for (double i = 0; i < .5; i += .01) {
            for (double j = .005; j < .02; j += .0001){
                Tester t = Tester("EUR_USD_M1", "EUR_USD_H1", i, j, &data, &dataLong);
                t.runMany();
                double profit = t.getTotal();
                std::cout << data.size() << " i: " << i << "  j: " << j << "  profit: " << profit << '\n';
                if (profit > bestProfit){
                    bestProfit = profit;
                    bestI = i;
                    bestJ = j;
                }

            }
        }

        std::cout << "I: " << bestI << "  J: " << bestJ << "  Profit: " << bestProfit << std::endl;

        return 0;
    }
    else {

        Tester t = Tester("EUR_USD_M1", "EUR_USD_H1", .005, .005);
        t.verbose = true;
        t.run();
        //std::cout << t.getTotal() << '\n';
    }

}
