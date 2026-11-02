#ifndef PORTFOLIO_PRICE_HISTORY_LOG_H
#define PORTFOLIO_PRICE_HISTORY_LOG_H

#include <string>
#include "portfolio/pricing/PriceQuote.h"

namespace portfolio {
namespace pricing {

class PriceHistoryLog {
private:
    std::string filename;

public:
    explicit PriceHistoryLog(const std::string& file);

    bool append(const PriceQuote& quote) const;
};

}
}

#endif