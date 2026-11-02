#ifndef PORTFOLIO_PRICE_QUOTE_H
#define PORTFOLIO_PRICE_QUOTE_H

#include <string>
#include "portfolio/core/Money.h"

namespace portfolio {
namespace pricing {

class PriceQuote {
private:
    std::string symbol;
    core::Money price;
    std::string quoteDate;
    std::string source;

public:
    PriceQuote()
        : symbol(""), price(), quoteDate(""), source("") {}

    PriceQuote(const std::string& stockSymbol,
               const core::Money& stockPrice,
               const std::string& date,
               const std::string& priceSource)
        : symbol(stockSymbol), price(stockPrice),
          quoteDate(date), source(priceSource) {}

    std::string getSymbol() const { return symbol; }
    core::Money getPrice() const { return price; }
    std::string getDate() const { return quoteDate; }
    std::string getSource() const { return source; }
};

}
}

#endif