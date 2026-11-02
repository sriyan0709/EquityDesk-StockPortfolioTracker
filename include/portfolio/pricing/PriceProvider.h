#ifndef PORTFOLIO_PRICE_PROVIDER_H
#define PORTFOLIO_PRICE_PROVIDER_H

#include <string>
#include "portfolio/pricing/PriceQuote.h"

namespace portfolio {
namespace pricing {

class PriceProvider {
public:
    virtual bool getPrice(const std::string& symbol,
                          PriceQuote& result) const = 0;

    virtual std::string getProviderName() const = 0;

    virtual ~PriceProvider() {}
};

}
}

#endif