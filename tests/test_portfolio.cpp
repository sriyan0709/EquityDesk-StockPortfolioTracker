#include <iostream>
#include <string>

#include "portfolio/portfolio/Portfolio.h"
#include "portfolio/transactions/BuyTransaction.h"
#include "portfolio/transactions/SellTransaction.h"
#include "portfolio/transactions/DividendTransaction.h"
#include "portfolio/transactions/SplitTransaction.h"
#include "portfolio/transactions/BonusTransaction.h"

using portfolio::core::Date;
using portfolio::core::Money;
using portfolio::portfolio_data::Portfolio;
using portfolio::transactions::BuyTransaction;
using portfolio::transactions::SellTransaction;
using portfolio::transactions::DividendTransaction;
using portfolio::transactions::SplitTransaction;
using portfolio::transactions::BonusTransaction;

namespace {
    int passed = 0;
    int failed = 0;

    void check(bool condition, const std::string& name) {
        if (condition) {
            ++passed;
            std::cout << "[PASS] " << name << '\n';
        } else {
            ++failed;
            std::cout << "[FAIL] " << name << '\n';
        }
    }
}

bool runPortfolioTests() {
    passed = 0;
    failed = 0;

    const Date buyDate(1, 1, 2025);
    const Date sellDate(1, 6, 2025);
    const Date dividendDate(1, 7, 2025);

    // TEST 1: FIFO accounting example from the project README.

    Portfolio portfolio;

    BuyTransaction buy(
        "TEST", 10, Money(1000), Money(20), buyDate
    );

    portfolio.applyTransaction(buy);

    check(
        portfolio.getTotalCost().getPaise() == 1002000,
        "Buy 10 shares at Rs.1000 with Rs.20 fees"
    );

    SellTransaction sell(
        "TEST", 4, Money(1200), Money(20), sellDate
    );

    portfolio.applyTransaction(sell);

    portfolio.setStockPrice("TEST", Money(1100));

    const portfolio::portfolio_data::Holding* holding =
        portfolio.findHolding("TEST");

    check(holding != 0, "Find existing stock holding");

    if (holding != 0) {
        check(
            holding->getQuantity() == 6,
            "FIFO sale leaves 6 shares"
        );

        check(
            holding->getTotalCost().getPaise() == 601200,
            "Remaining purchase cost is Rs.6012"
        );

        check(
            holding->getRealisedProfitLoss().getPaise() == 77200,
            "Realised profit/loss is Rs.772"
        );

        check(
            holding->getMarketValue().getPaise() == 660000,
            "Market value is Rs.6600"
        );

        check(
            holding->getUnrealisedProfitLoss().getPaise() == 58800,
            "Unrealised profit/loss is Rs.588"
        );
    }

    // TEST 2: Dividend income.

    DividendTransaction dividend(
        "TEST", 6, Money(5), dividendDate
    );

    portfolio.applyTransaction(dividend);

    check(
        portfolio.getTotalDividendIncome().getPaise() == 3000,
        "Dividend income is Rs.30"
    );

    check(
        portfolio.getTotalReturn().getPaise() == 139000,
        "Total return is Rs.1390"
    );

    // TEST 3: Multiple stocks.

    BuyTransaction secondBuy(
        "OTHER", 3, Money(200), Money(0), buyDate
    );

    portfolio.applyTransaction(secondBuy);

    check(
        portfolio.getHoldingCount() == 2,
        "Portfolio tracks multiple stocks"
    );

    check(
        portfolio.getTotalShareCount() == 9,
        "Total share count across stocks is 9"
    );

    // TEST 4: Reject selling more shares than owned.

    bool insufficientSharesCaught = false;

    try {
        SellTransaction invalidSell(
            "TEST", 100, Money(1200), Money(0), sellDate
        );

        portfolio.applyTransaction(invalidSell);
    }
    catch (const ::portfolio::InsufficientSharesException&) {
        insufficientSharesCaught = true;
    }

    check(
        insufficientSharesCaught,
        "Reject sale when shares are insufficient"
    );

    // TEST 5: Stock split.

    Portfolio splitPortfolio;

    BuyTransaction splitBuy(
        "SPLT", 10, Money(100), Money(0), buyDate
    );

    splitPortfolio.applyTransaction(splitBuy);

    SplitTransaction split(
        "SPLT", 2, 1, sellDate
    );

    splitPortfolio.applyTransaction(split);

    const portfolio::portfolio_data::Holding* splitHolding =
        splitPortfolio.findHolding("SPLT");

    check(
        splitHolding != 0 && splitHolding->getQuantity() == 20,
        "2-for-1 split doubles the share quantity"
    );

    check(
        splitHolding != 0 &&
        splitHolding->getTotalCost().getPaise() == 100000,
        "Stock split preserves total cost basis"
    );

    // TEST 6: Bonus shares.

    BonusTransaction bonus(
        "SPLT", 1, 1, dividendDate
    );

    splitPortfolio.applyTransaction(bonus);

    splitHolding = splitPortfolio.findHolding("SPLT");

    check(
        splitHolding != 0 && splitHolding->getQuantity() == 40,
        "1-for-1 bonus doubles the share quantity"
    );

    check(
        splitHolding != 0 &&
        splitHolding->getTotalCost().getPaise() == 100000,
        "Bonus issue preserves total cost basis"
    );

    std::cout << "\n----------------------------------------\n";
    std::cout << "Passed: " << passed
              << " | Failed: " << failed << '\n';
    std::cout << "----------------------------------------\n";

    return failed == 0;
}