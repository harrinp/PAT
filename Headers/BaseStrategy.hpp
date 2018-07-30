#ifndef BASESTRATEGY_HPP
#define BASESTRATEGY_HPP

#include <string>
#include <unistd.h>

#include "Decision.hpp"
#include "Trade.hpp"

/*
 *  This is the base strategy class to be extended by all other strategies
 */
class BaseStrategy {
private:
    std::string tableName;

public:
    Decider::Pos eval(Price p);
    BaseStrategy ();
    virtual ~BaseStrategy ();
};


#endif
