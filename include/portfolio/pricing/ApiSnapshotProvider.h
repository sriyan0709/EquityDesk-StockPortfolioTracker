#ifndef PORTFOLIO_API_SNAPSHOT_PROVIDER_H
#define PORTFOLIO_API_SNAPSHOT_PROVIDER_H

#include <string>
#include "portfolio/pricing/CsvPriceProvider.h"

namespace portfolio {
namespace pricing {

// Reads a normalized CSV snapshot produced by an API-fetching script.
// This class does not assume a particular third-party JSON response format.

class ApiSnapshotProvider : public CsvPriceProvider {
public:
    explicit ApiSnapshotProvider(const std::string& snapshotFile);

    std::string getProviderName() const;
};

}
}

#endif