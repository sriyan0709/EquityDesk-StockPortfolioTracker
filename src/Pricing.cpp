#include "portfolio/pricing/CsvPriceProvider.h"
#include "portfolio/pricing/ManualPriceProvider.h"
#include "portfolio/pricing/ApiSnapshotProvider.h"
#include "portfolio/pricing/PriceHistoryLog.h"
#include "portfolio/core/Exceptions.h"
#include "portfolio/core/Date.h"

#include <fstream>
#include <sstream>
#include <string>
#include <cctype>

namespace portfolio {
namespace pricing {

namespace {

std::string trim(const std::string& value) {
    std::string::size_type first = 0;
    std::string::size_type last = value.size();

    while (first < last &&
           std::isspace(static_cast<unsigned char>(value[first]))) {
        ++first;
    }

    while (last > first &&
           std::isspace(static_cast<unsigned char>(value[last - 1]))) {
        --last;
    }

    return value.substr(first, last - first);
}

std::string upper(const std::string& value) {
    std::string result = trim(value);

    for (std::size_t i = 0; i < result.size(); ++i) {
        result[i] = static_cast<char>(
            std::toupper(static_cast<unsigned char>(result[i]))
        );
    }

    return result;
}

bool validSymbol(const std::string& symbol) {
    if (symbol.empty() || symbol.size() > 20) {
        return false;
    }

    for (std::size_t i = 0; i < symbol.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(symbol[i]);

        if (!std::isalnum(c) && c != '.' && c != '&' && c != '-') {
            return false;
        }
    }

    return true;
}

bool validDateText(const std::string& value) {
    if (value.size() != 10 ||
        value[4] != '-' || value[7] != '-') {
        return false;
    }

    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) {
            continue;
        }

        if (!std::isdigit(static_cast<unsigned char>(value[i]))) {
            return false;
        }
    }

    std::istringstream input(value);
    core::Date date;
    input >> date;

    return !input.fail() && date.isValid();
}

bool parsePrice(const std::string& text, core::Money& price) {
    std::istringstream input(trim(text));
    input >> price;

    if (input.fail()) {
        return false;
    }

    input >> std::ws;
    return input.eof() && price.getPaise() >= 0;
}

}

CsvPriceProvider::CsvPriceProvider(const std::string& file)
    : filename(file) {
}

bool CsvPriceProvider::getPrice(
    const std::string& requestedSymbol,
    PriceQuote& result
) const {
    std::string wanted = upper(requestedSymbol);

    if (!validSymbol(wanted)) {
        throw InvalidSymbolException("Invalid stock symbol.");
    }

    std::ifstream file(filename.c_str());

    if (!file.is_open()) {
        throw PriceUnavailableException(
            "Unable to open price CSV file: " + filename
        );
    }

    std::string line;

    while (std::getline(file, line)) {
        if (trim(line).empty()) {
            continue;
        }

        std::string::size_type comma = line.find(',');

        if (comma == std::string::npos) {
            continue;
        }

        std::string fileSymbol = upper(line.substr(0, comma));
        std::string priceText = trim(line.substr(comma + 1));

        if (fileSymbol == "SYMBOL") {
            continue;
        }

        if (fileSymbol != wanted) {
            continue;
        }

        core::Money price;

        if (!parsePrice(priceText, price)) {
            throw PriceUnavailableException(
                "Invalid price for " + wanted + " in " + filename
            );
        }

        result = PriceQuote(
            wanted, price, "", getProviderName()
        );

        return true;
    }

    return false;
}

std::string CsvPriceProvider::getProviderName() const {
    return "CSV";
}

// ------------------------------------------------------------

ManualPriceProvider::ManualPriceProvider()
    : quotes() {
}

void ManualPriceProvider::setPrice(
    const std::string& requestedSymbol,
    const core::Money& price,
    const std::string& date
) {
    std::string symbol = upper(requestedSymbol);

    if (!validSymbol(symbol)) {
        throw InvalidSymbolException("Invalid stock symbol.");
    }

    if (price.getPaise() <= 0) {
        throw InvalidTransactionException(
            "Manual stock price must be greater than zero."
        );
    }

    if (!validDateText(date)) {
        throw InvalidDateException(
            "Date must be a valid date in YYYY-MM-DD format."
        );
    }

    PriceQuote quote(symbol, price, date, getProviderName());

    for (int i = 0; i < quotes.size(); ++i) {
        if (quotes.at(i).getSymbol() == symbol) {
            quotes.at(i) = quote;
            return;
        }
    }

    quotes.pushBack(quote);
}

bool ManualPriceProvider::getPrice(
    const std::string& requestedSymbol,
    PriceQuote& result
) const {
    std::string wanted = upper(requestedSymbol);

    if (!validSymbol(wanted)) {
        throw InvalidSymbolException("Invalid stock symbol.");
    }

    for (int i = 0; i < quotes.size(); ++i) {
        if (quotes.at(i).getSymbol() == wanted) {
            result = quotes.at(i);
            return true;
        }
    }

    return false;
}

std::string ManualPriceProvider::getProviderName() const {
    return "MANUAL";
}

int ManualPriceProvider::getQuoteCount() const {
    return quotes.size();
}

// ------------------------------------------------------------

ApiSnapshotProvider::ApiSnapshotProvider(
    const std::string& snapshotFile
) : CsvPriceProvider(snapshotFile) {
}

std::string ApiSnapshotProvider::getProviderName() const {
    return "API_SNAPSHOT";
}

// ------------------------------------------------------------

PriceHistoryLog::PriceHistoryLog(const std::string& file)
    : filename(file) {
}

bool PriceHistoryLog::append(const PriceQuote& quote) const {
    std::ofstream file(filename.c_str(), std::ios::app);

    if (!file.is_open()) {
        return false;
    }

    file << quote.getDate() << ","
         << quote.getSymbol() << ","
         << quote.getPrice().getPaise() << ","
         << quote.getSource() << "\n";

    return file.good();
}

}
}