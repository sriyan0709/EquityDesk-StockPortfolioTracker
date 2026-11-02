#ifndef PORTFOLIO_PORTFOLIO_H
#define PORTFOLIO_PORTFOLIO_H

#include <string>

#include "portfolio/core/Exceptions.h"
#include "portfolio/core/Money.h"
#include "portfolio/templates/DynamicArray.h"
#include "portfolio/transactions/Transaction.h"
#include "portfolio/portfolio/Holding.h"

namespace portfolio {
namespace portfolio_data {

class Portfolio : public transactions::TransactionActions {
private:
    templates::DynamicArray<Holding> holdings;

    int findHoldingIndex(const std::string& symbol) const;
    Holding& getOrCreateHolding(const std::string& symbol);
    Holding& requireHolding(const std::string& symbol);

public:
    Portfolio();

    int getHoldingCount() const;
    int getTotalShareCount() const;

    Holding* findHolding(const std::string& symbol);
    const Holding* findHolding(const std::string& symbol) const;

    Holding& getHoldingAt(int index);
    const Holding& getHoldingAt(int index) const;

    void setStockPrice(const std::string& symbol,
                       const core::Money& price);

    core::Money getTotalCost() const;
    core::Money getTotalMarketValue() const;
    core::Money getTotalRealisedProfitLoss() const;
    core::Money getTotalUnrealisedProfitLoss() const;
    core::Money getTotalDividendIncome() const;
    core::Money getTotalReturn() const;

    void applyTransaction(
        const transactions::Transaction& transaction
    );

    void recordBuy(
        const std::string& symbol,
        int quantity,
        const core::Money& price,
        const core::Money& fees,
        const core::Date& date,
        long transactionId
    );

    void recordSell(
        const std::string& symbol,
        int quantity,
        const core::Money& price,
        const core::Money& fees,
        const core::Date& date,
        long transactionId
    );

    void recordDividend(
        const std::string& symbol,
        int quantity,
        const core::Money& amountPerShare,
        const core::Date& date,
        long transactionId
    );

    void recordSplit(
        const std::string& symbol,
        int numerator,
        int denominator,
        const core::Date& date,
        long transactionId
    );

    void recordBonus(
        const std::string& symbol,
        int numerator,
        int denominator,
        const core::Date& date,
        long transactionId
    );
};

}
}

#endif