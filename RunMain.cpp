#include "Headers/Decision.hpp"

int main(int argc, char *argv[]) {
    Runner r = Runner("EUR_USD_M1", "EUR_USD_H1", "EUR_USD", .005, .005);
    r.run();
    return 0;
}
