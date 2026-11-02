#ifndef PORTFOLIO_CSV_PRICE_PROVIDER_H
#define PORTFOLIO_CSV_PRICE_PROVIDER_H

#include <string>
#include "portfolio/pricing/PriceProvider.h"

namespace portfolio {
namespace pricing {

class CsvPriceProvider : public PriceProvider {
private:
    std::string filename;

public:
    explicit CsvPriceProvider(const std::string& file);

    bool getPrice(const std::string& symbol,
                  PriceQuote& result) const;

    std::string getProviderName() const;
};

}
}

#endif