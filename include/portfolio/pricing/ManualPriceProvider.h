#ifndef PORTFOLIO_MANUAL_PRICE_PROVIDER_H
#define PORTFOLIO_MANUAL_PRICE_PROVIDER_H

#include <string>
#include "portfolio/pricing/PriceProvider.h"
#include "portfolio/templates/DynamicArray.h"

namespace portfolio {
namespace pricing {

class ManualPriceProvider : public PriceProvider {
private:
    templates::DynamicArray<PriceQuote> quotes;

public:
    ManualPriceProvider();

    void setPrice(const std::string& symbol,
                  const core::Money& price,
                  const std::string& date);

    bool getPrice(const std::string& symbol,
                  PriceQuote& result) const;

    std::string getProviderName() const;
    int getQuoteCount() const;
};

}
}

#endif