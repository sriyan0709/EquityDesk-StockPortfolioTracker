#include <iostream>
#include <cstdio>
#include <fstream>

#include "portfolio/storage/TransactionStore.h"
#include "portfolio/portfolio/Portfolio.h"

#include "portfolio/transactions/BuyTransaction.h"
#include "portfolio/transactions/SellTransaction.h"
#include "portfolio/transactions/DividendTransaction.h"

using portfolio::core::Date;
using portfolio::core::Money;

using portfolio::storage::TransactionStore;

using portfolio::transactions::BuyTransaction;
using portfolio::transactions::SellTransaction;
using portfolio::transactions::DividendTransaction;

bool runStorageTests() {

    int passed = 0;
    int failed = 0;

    const std::string filename =
        "data/storage_test.dat";

    TransactionStore original;

    BuyTransaction buy(
        "TCS",
        10,
        Money(100),
        Money(5),
        Date(1, 1, 2025)
    );

    SellTransaction sell(
        "TCS",
        2,
        Money(120),
        Money(5),
        Date(1, 2, 2025)
    );

    DividendTransaction dividend(
        "TCS",
        8,
        Money(3),
        Date(1, 3, 2025)
    );

    original.add(buy);
    original.add(sell);
    original.add(dividend);

    if (original.size() == 3) {
        ++passed;
        std::cout << "[PASS] Three transactions added\n";
    } else {
        ++failed;
        std::cout << "[FAIL] Adding transactions\n";
    }

    if (original.saveToFile(filename)) {
        ++passed;
        std::cout << "[PASS] Transactions saved to file\n";
    } else {
        ++failed;
        std::cout << "[FAIL] Saving transactions\n";
    }

    TransactionStore loaded;

    if (loaded.loadFromFile(filename)) {
        ++passed;
        std::cout << "[PASS] Transactions loaded from file\n";
    } else {
        ++failed;
        std::cout << "[FAIL] Loading transactions\n";
    }

    if (loaded.size() == 3) {
        ++passed;
        std::cout << "[PASS] Loaded transaction count is correct\n";
    } else {
        ++failed;
        std::cout << "[FAIL] Loaded transaction count\n";
    }

    if (loaded.size() == 3) {

        const portfolio::storage::TransactionRecord& first =
            loaded.get(0);

        if (first.symbol == "TCS" &&
            first.quantity == 10 &&
            first.pricePaise == 10000) {

            ++passed;
            std::cout << "[PASS] Buy record data preserved\n";

        } else {
            ++failed;
            std::cout << "[FAIL] Buy record data\n";
        }
    }

    if (loaded.size() == 3) {

        portfolio::transactions::Transaction* restored =
            loaded.createTransaction(0);

        if (restored != 0 &&
            restored->getType() == portfolio::transactions::BUY) {

            ++passed;
            std::cout << "[PASS] Buy transaction reconstructed\n";

        } else {
            ++failed;
            std::cout << "[FAIL] Buy transaction reconstruction\n";
        }

        delete restored;
    }

    if (loaded.size() == 3) {

        portfolio::transactions::Transaction* restored =
            loaded.createTransaction(1);

        if (restored != 0 &&
            restored->getType() == portfolio::transactions::SELL) {

            ++passed;
            std::cout << "[PASS] Sell transaction reconstructed\n";

        } else {
            ++failed;
            std::cout << "[FAIL] Sell transaction reconstruction\n";
        }

        delete restored;
    }

    if (loaded.size() == 3) {

        portfolio::transactions::Transaction* restored =
            loaded.createTransaction(2);

        if (restored != 0 &&
            restored->getType() == portfolio::transactions::DIVIDEND) {

            ++passed;
            std::cout << "[PASS] Dividend transaction reconstructed\n";

        } else {
            ++failed;
            std::cout << "[FAIL] Dividend transaction reconstruction\n";
        }

        delete restored;
    }

    // Invalid file format test.

    const std::string invalidFilename =
        "data/invalid_test.dat";

    {
        std::ofstream invalidFile(
            invalidFilename.c_str()
        );

        invalidFile << "INVALID_HEADER\n";
    }

    TransactionStore invalidStore;

    if (!invalidStore.loadFromFile(invalidFilename)) {
        ++passed;
        std::cout << "[PASS] Invalid file format rejected\n";
    } else {
        ++failed;
        std::cout << "[FAIL] Invalid file format validation\n";
    }

    // Chronological replay test.

    TransactionStore outOfOrder;

    BuyTransaction earlierBuy(
        "INFY",
        10,
        Money(100),
        Money(0),
        Date(1, 1, 2025)
    );

    SellTransaction laterSell(
        "INFY",
        2,
        Money(120),
        Money(0),
        Date(3, 1, 2025)
    );

    // Deliberately save the sale first.
    outOfOrder.add(laterSell);
    outOfOrder.add(earlierBuy);

    portfolio::portfolio_data::Portfolio restoredPortfolio;

    outOfOrder.replayInto(restoredPortfolio);

    const portfolio::portfolio_data::Holding* restoredHolding =
        restoredPortfolio.findHolding("INFY");

    if (restoredHolding != 0 &&
        restoredHolding->getQuantity() == 8) {

        ++passed;
        std::cout << "[PASS] Replay sorts transactions chronologically\n";

    } else {
        ++failed;
        std::cout << "[FAIL] Chronological replay\n";
    }

    if (restoredHolding != 0 &&
        restoredHolding->getTotalCost().getPaise() == 80000 &&
        restoredHolding->getRealisedProfitLoss().getPaise() == 4000) {

        ++passed;
        std::cout << "[PASS] Replayed portfolio accounting matches expected values\n";

    } else {
        ++failed;
        std::cout << "[FAIL] Replayed portfolio accounting\n";
    }

    if (restoredHolding != 0 &&
        restoredHolding->getLotCount() > 0 &&
        restoredHolding->getLot(0).getTransactionId() ==
            outOfOrder.get(1).transactionId) {

        ++passed;
        std::cout << "[PASS] Replay preserves original purchase transaction ID\n";

    } else {
        ++failed;
        std::cout << "[FAIL] Original transaction ID preservation\n";
    }

    // New transaction IDs must continue beyond restored IDs.

    BuyTransaction transactionAfterRestore(
        "RELIANCE",
        1,
        Money(10),
        Money(0),
        Date(4, 1, 2025)
    );

    long largestSavedId = outOfOrder.get(0).transactionId;

    if (outOfOrder.get(1).transactionId > largestSavedId) {
        largestSavedId = outOfOrder.get(1).transactionId;
    }

    if (transactionAfterRestore.getId() > largestSavedId) {

        ++passed;
        std::cout << "[PASS] New transactions receive IDs beyond restored history\n";

    } else {
        ++failed;
        std::cout << "[FAIL] Transaction ID sequence after replay\n";
    }

    // Failed replay must not overwrite an existing portfolio.

    TransactionStore invalidSequence;

    SellTransaction impossibleSale(
        "WIPRO",
        5,
        Money(50),
        Money(0),
        Date(5, 1, 2025)
    );

    invalidSequence.add(impossibleSale);

    portfolio::portfolio_data::Portfolio unchanged;

    BuyTransaction existingBuy(
        "HCL",
        3,
        Money(100),
        Money(0),
        Date(1, 1, 2025)
    );

    unchanged.applyTransaction(existingBuy);

    bool replayRejected = false;

    try {
        invalidSequence.replayInto(unchanged);
    }
    catch (const portfolio::InsufficientSharesException&) {
        replayRejected = true;
    }
    catch (const portfolio::PortfolioException&) {
        replayRejected = true;
    }

    if (replayRejected &&
        unchanged.findHolding("HCL") != 0 &&
        unchanged.findHolding("HCL")->getQuantity() == 3 &&
        unchanged.findHolding("WIPRO") == 0) {

        ++passed;
        std::cout << "[PASS] Failed replay leaves existing portfolio unchanged\n";

    } else {
        ++failed;
        std::cout << "[FAIL] Replay atomicity on invalid transaction sequence\n";
    }

    std::remove(filename.c_str());
    std::remove(invalidFilename.c_str());

    std::cout << "\nStorage tests passed: "
              << passed
              << " | Failed: "
              << failed
              << '\n';

    return failed == 0;
}