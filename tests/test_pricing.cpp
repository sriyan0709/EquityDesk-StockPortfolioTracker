#include <iostream>
#include <fstream>
#include <cstdio>

#include "portfolio/pricing/CsvPriceProvider.h"
#include "portfolio/pricing/ManualPriceProvider.h"
#include "portfolio/pricing/ApiSnapshotProvider.h"
#include "portfolio/pricing/PriceHistoryLog.h"
#include "portfolio/core/Exceptions.h"

using namespace portfolio;
using namespace portfolio::pricing;
using namespace portfolio::core;

namespace {

int passed = 0;
int failed = 0;

void check(bool condition, const char* description) {
    if (condition) {
        ++passed;
        std::cout << "[PASS] " << description << "\n";
    } else {
        ++failed;
        std::cout << "[FAIL] " << description << "\n";
    }
}

}

bool runPricingTests() {
    passed = 0;
    failed = 0;

    std::cout << "\n========== PRICING TESTS ==========\n\n";

    const char* csvFile = "pricing_test.csv";
    const char* historyFile = "pricing_history_test.csv";

    {
        std::ofstream file(csvFile);

        file << "symbol,price\n";
        file << "TCS,3456.70\n";
        file << "INFY,1512.25\n";
        file << "RELIANCE,1320.00\n";
    }

    // Test 1: CSV provider loads an existing symbol.
    CsvPriceProvider csv(csvFile);
    PriceQuote quote;

    bool found = csv.getPrice("TCS", quote);

    check(
        found && quote.getPrice().getPaise() == 345670,
        "CSV provider reads a stock price correctly"
    );

    // Test 2: Symbol matching is case-insensitive.
    found = csv.getPrice("infy", quote);

    check(
        found && quote.getSymbol() == "INFY" &&
        quote.getPrice().getPaise() == 151225,
        "CSV provider handles lowercase symbol input"
    );

    // Test 3: Missing symbol returns false.
    found = csv.getPrice("WIPRO", quote);

    check(
        !found,
        "CSV provider reports a missing stock symbol"
    );

    // Test 4: Provider identifies its source.
    check(
        csv.getProviderName() == "CSV",
        "CSV provider returns its provider name"
    );

    // Test 5: Manual provider stores a quote.
    ManualPriceProvider manual;

    manual.setPrice(
        "HDFCBANK",
        Money(1650, 50),
        "2026-11-01"
    );

    found = manual.getPrice("HDFCBANK", quote);

    check(
        found && quote.getPrice().getPaise() == 165050,
        "Manual provider stores and retrieves a price"
    );

    // Test 6: Manual provider updates an existing quote.
    manual.setPrice(
        "HDFCBANK",
        Money(1700, 25),
        "2026-11-02"
    );

    found = manual.getPrice("HDFCBANK", quote);

    check(
        found && manual.getQuoteCount() == 1 &&
        quote.getPrice().getPaise() == 170025 &&
        quote.getDate() == "2026-11-02",
        "Manual provider updates an existing stock quote"
    );

    // Test 7: Manual provider rejects a zero price.
    bool rejectedPrice = false;

    try {
        manual.setPrice("TCS", Money(0), "2026-11-01");
    } catch (const InvalidTransactionException&) {
        rejectedPrice = true;
    }

    check(
        rejectedPrice,
        "Manual provider rejects a zero stock price"
    );

    // Test 8: Manual provider rejects an invalid date.
    bool rejectedDate = false;

    try {
        manual.setPrice("TCS", Money(100), "2026-15-40");
    } catch (const InvalidDateException&) {
        rejectedDate = true;
    }

    check(
        rejectedDate,
        "Manual provider rejects an invalid date"
    );

    // Test 9: API snapshot reads the normalized CSV format.
    ApiSnapshotProvider api(csvFile);

    found = api.getPrice("RELIANCE", quote);

    check(
        found &&
        quote.getPrice().getPaise() == 132000 &&
        quote.getSource() == "API_SNAPSHOT",
        "API snapshot provider reads a normalized snapshot"
    );

    // Test 10: API snapshot identifies its source.
    check(
        api.getProviderName() == "API_SNAPSHOT",
        "API snapshot provider returns its provider name"
    );

    // Test 11: Price history appends a record.
    std::remove(historyFile);

    PriceHistoryLog history(historyFile);

    PriceQuote historyQuote(
        "TCS",
        Money(3456, 70),
        "2026-11-01",
        "MANUAL"
    );

    bool logged = history.append(historyQuote);

    std::ifstream historyInput(historyFile);
    std::string historyLine;
    std::getline(historyInput, historyLine);

    check(
        logged &&
        historyLine == "2026-11-01,TCS,345670,MANUAL",
        "Price history log writes a price record"
    );

    // Test 12: History log can append multiple records.
    PriceQuote secondHistoryQuote(
        "INFY",
        Money(1512, 25),
        "2026-11-02",
        "CSV"
    );

    history.append(secondHistoryQuote);

    std::ifstream secondInput(historyFile);
    int lineCount = 0;
    std::string line;

    while (std::getline(secondInput, line)) {
        ++lineCount;
    }

    check(
        lineCount == 2,
        "Price history log appends multiple records"
    );

    // Test 13: Missing CSV file is reported.
    bool missingFileRejected = false;

    try {
        CsvPriceProvider missing("file_that_does_not_exist.csv");
        missing.getPrice("TCS", quote);
    } catch (const PriceUnavailableException&) {
        missingFileRejected = true;
    }

    check(
        missingFileRejected,
        "CSV provider reports a missing price file"
    );

    std::remove(csvFile);
    std::remove(historyFile);

    std::cout << "\nPricing tests passed: " << passed
              << " | Failed: " << failed << "\n";

    return failed == 0;
}