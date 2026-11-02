
#include <iostream>
#include <string>
#include <limits>
#include <ctime>
#include <iomanip>

#include "portfolio/core/Date.h"
#include "portfolio/core/Money.h"
#include "portfolio/core/Exceptions.h"

#include "portfolio/portfolio/Portfolio.h"
#include "portfolio/portfolio/Holding.h"

#include "portfolio/storage/TransactionStore.h"

#include "portfolio/pricing/CsvPriceProvider.h"
#include "portfolio/pricing/ManualPriceProvider.h"
#include "portfolio/pricing/PriceHistoryLog.h"
#include "portfolio/pricing/PriceQuote.h"

using namespace portfolio;
using namespace portfolio::core;
using namespace portfolio::portfolio_data;
using namespace portfolio::storage;
using namespace portfolio::pricing;

namespace {

const std::string TRANSACTION_FILE = "data/transactions.dat";
const std::string PRICE_FILE = "data/prices.csv";
const std::string HISTORY_FILE = "data/price_history.csv";

// Returns today's date in YYYY-MM-DD format.
std::string getTodayDate() {
    std::time_t currentTime = std::time(NULL);
    std::tm* localTime = std::localtime(&currentTime);

    if (localTime == NULL) {
        return "2026-11-01";
    }

    char buffer[11];

    std::strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d",
        localTime
    );

    return std::string(buffer);
}

void clearInput() {
    std::cin.clear();
    std::cin.ignore(
        std::numeric_limits<std::streamsize>::max(),
        '\n'
    );
}

int readChoice() {
    int choice;

    std::cout << "\nEnter your choice: ";

    if (!(std::cin >> choice)) {
        clearInput();
        return -1;
    }

    clearInput();
    return choice;
}

void displayHeading() {
    std::cout << "\n========================================\n";
    std::cout << "       STOCK PORTFOLIO TRACKER\n";
    std::cout << "========================================\n";
}

void displayMenu() {
    std::cout << "\n--------------- MAIN MENU ---------------\n";
    std::cout << "1. Refresh prices from CSV\n";
    std::cout << "2. Enter or update a price manually\n";
    std::cout << "3. View all holdings\n";
    std::cout << "4. View portfolio summary\n";
    std::cout << "5. View saved transaction count\n";
    std::cout << "0. Exit\n";
    std::cout << "-----------------------------------------\n";
}

void displayHoldings(const Portfolio& portfolio) {
    if (portfolio.getHoldingCount() == 0) {
        std::cout << "\nYour portfolio has no holdings.\n";
        std::cout << "Load saved transactions before refreshing prices.\n";
        return;
    }

    std::cout << "\n================ HOLDINGS ================\n";

    std::cout << std::left
              << std::setw(16) << "SYMBOL"
              << std::setw(12) << "QUANTITY"
              << std::setw(16) << "AVG COST"
              << std::setw(16) << "LAST PRICE"
              << std::setw(16) << "MARKET VALUE"
              << "UNREALISED P/L\n";

    std::cout << "------------------------------------------------"
              << "----------------------------------------\n";

    for (int i = 0; i < portfolio.getHoldingCount(); ++i) {
        const Holding& holding = portfolio.getHoldingAt(i);

        std::cout << std::left
                  << std::setw(16) << holding.getSymbol()
                  << std::setw(12) << holding.getQuantity()
                  << std::setw(16)
                  << holding.getAverageCostPerShare().toString()
                  << std::setw(16)
                  << holding.getLastPrice().toString()
                  << std::setw(16)
                  << holding.getMarketValue().toString()
                  << holding.getUnrealisedProfitLoss().toString()
                  << "\n";
    }

    std::cout << "===========================================\n";
}

void displaySummary(const Portfolio& portfolio) {
    std::cout << "\n========== PORTFOLIO SUMMARY ==========\n";

    std::cout << "Number of holdings: "
              << portfolio.getHoldingCount() << "\n";

    std::cout << "Total shares: "
              << portfolio.getTotalShareCount() << "\n";

    std::cout << "Total invested cost: Rs. "
              << portfolio.getTotalCost() << "\n";

    std::cout << "Current market value: Rs. "
              << portfolio.getTotalMarketValue() << "\n";

    std::cout << "Realised profit/loss: Rs. "
              << portfolio.getTotalRealisedProfitLoss() << "\n";

    std::cout << "Unrealised profit/loss: Rs. "
              << portfolio.getTotalUnrealisedProfitLoss() << "\n";

    std::cout << "Dividend income: Rs. "
              << portfolio.getTotalDividendIncome() << "\n";

    std::cout << "Total return: Rs. "
              << portfolio.getTotalReturn() << "\n";

    std::cout << "=======================================\n";
}

void refreshPrices(
    Portfolio& portfolio,
    const CsvPriceProvider& provider,
    const PriceHistoryLog& history
) {
    if (portfolio.getHoldingCount() == 0) {
        std::cout << "\nNo holdings available to refresh.\n";
        return;
    }

    int updated = 0;
    int unavailable = 0;

    std::string today = getTodayDate();

    std::cout << "\nRefreshing stock prices...\n";

    for (int i = 0; i < portfolio.getHoldingCount(); ++i) {
        const Holding& holding = portfolio.getHoldingAt(i);

        std::string symbol = holding.getSymbol();

        try {
            PriceQuote quote;

            if (provider.getPrice(symbol, quote)) {
                portfolio.setStockPrice(
                    symbol,
                    quote.getPrice()
                );

                PriceQuote historyQuote(
                    symbol,
                    quote.getPrice(),
                    today,
                    quote.getSource()
                );

                if (history.append(historyQuote)) {
                    std::cout << "[UPDATED] "
                              << symbol
                              << " : Rs. "
                              << quote.getPrice()
                              << "\n";
                } else {
                    std::cout << "[UPDATED, NOT LOGGED] "
                              << symbol << "\n";
                }

                ++updated;
            } else {
                std::cout << "[NOT FOUND] "
                          << symbol
                          << " is missing from the CSV file.\n";

                ++unavailable;
            }
        }
        catch (const PortfolioException& error) {
            std::cout << "[ERROR] "
                      << symbol << " : "
                      << error.what() << "\n";

            ++unavailable;
        }
    }

    std::cout << "\n========== REFRESH RESULT ==========\n";
    std::cout << "Prices updated: " << updated << "\n";
    std::cout << "Prices unavailable: " << unavailable << "\n";
    std::cout << "====================================\n";
}

void enterManualPrice(
    Portfolio& portfolio,
    ManualPriceProvider& provider,
    const PriceHistoryLog& history
) {
    std::string symbol;
    std::string date;
    Money price;

    std::cout << "\nEnter stock symbol: ";
    std::getline(std::cin, symbol);

    if (portfolio.findHolding(symbol) == NULL) {
        std::cout << "This stock is not present in your portfolio.\n";
        return;
    }

    std::cout << "Enter current price in rupees: ";

    if (!(std::cin >> price)) {
        clearInput();
        std::cout << "Invalid price input.\n";
        return;
    }

    clearInput();

    std::cout << "Enter date (YYYY-MM-DD): ";
    std::getline(std::cin, date);

    try {
        provider.setPrice(symbol, price, date);

        PriceQuote quote;

        if (provider.getPrice(symbol, quote)) {
            portfolio.setStockPrice(
                quote.getSymbol(),
                quote.getPrice()
            );

            if (history.append(quote)) {
                std::cout << "\nPrice updated and saved in history.\n";
            } else {
                std::cout << "\nPrice updated, but history could not be saved.\n";
            }

            std::cout << "Stock: " << quote.getSymbol() << "\n";
            std::cout << "New price: Rs. "
                      << quote.getPrice() << "\n";
        }
    }
    catch (const PortfolioException& error) {
        std::cout << "\nUnable to update price: "
                  << error.what() << "\n";
    }
}

void loadSavedPortfolio(
    TransactionStore& store,
    Portfolio& portfolio
) {
    std::cout << "\nLoading saved transactions...\n";

    if (!store.loadFromFile(TRANSACTION_FILE)) {
        std::cout << "No saved transaction file was loaded.\n";
        std::cout << "You can still use the pricing menu.\n";
        return;
    }

    if (store.isEmpty()) {
        std::cout << "The transaction file is empty.\n";
        return;
    }

    store.replayInto(portfolio);

    std::cout << "Transactions loaded: "
              << store.size() << "\n";

    std::cout << "Portfolio replay completed successfully.\n";
}

}

int main() {
    try {
        displayHeading();

        Portfolio portfolio;
        TransactionStore store;

        CsvPriceProvider csvProvider(PRICE_FILE);
        ManualPriceProvider manualProvider;
        PriceHistoryLog history(HISTORY_FILE);

        loadSavedPortfolio(store, portfolio);

        int choice;

        do {
            displayMenu();
            choice = readChoice();

            try {
                switch (choice) {
                    case 1:
                        refreshPrices(
                            portfolio,
                            csvProvider,
                            history
                        );
                        break;

                    case 2:
                        enterManualPrice(
                            portfolio,
                            manualProvider,
                            history
                        );
                        break;

                    case 3:
                        displayHoldings(portfolio);
                        break;

                    case 4:
                        displaySummary(portfolio);
                        break;

                    case 5:
                        std::cout << "\nSaved transactions: "
                                  << store.size() << "\n";
                        break;

                    case 0:
                        std::cout << "\nExiting Stock Portfolio Tracker.\n";
                        break;

                    case -1:
                        std::cout << "\nInvalid menu input. Enter a number.\n";
                        break;

                    default:
                        std::cout << "\nInvalid choice. Try again.\n";
                }
            }
            catch (const PortfolioException& error) {
                std::cout << "\nApplication error: "
                          << error.what() << "\n";
            }

        } while (choice != 0);

    }
    catch (const PortfolioException& error) {
        std::cout << "\nFatal project error: "
                  << error.what() << "\n";

        return 1;
    }
    catch (const std::exception& error) {
        std::cout << "\nUnexpected error: "
                  << error.what() << "\n";

        return 1;
    }

    return 0;
}
