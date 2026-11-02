#include "portfolio/portfolio/Portfolio.h"

namespace portfolio {
namespace portfolio_data {

Portfolio::Portfolio()
    : holdings() {
}

int Portfolio::findHoldingIndex(
    const std::string& symbol
) const {
    for (int i = 0; i < holdings.size(); ++i) {
        if (holdings.at(i).getSymbol() == symbol) {
            return i;
        }
    }

    return -1;
}

Holding& Portfolio::getOrCreateHolding(
    const std::string& symbol
) {
    if (symbol.empty()) {
        throw ::portfolio::InvalidSymbolException(
            "Stock symbol cannot be empty."
        );
    }

    int index = findHoldingIndex(symbol);

    if (index >= 0) {
        return holdings.at(index);
    }

    holdings.pushBack(Holding(symbol));

    return holdings.at(holdings.size() - 1);
}

Holding& Portfolio::requireHolding(
    const std::string& symbol
) {
    int index = findHoldingIndex(symbol);

    if (index < 0) {
        throw ::portfolio::InvalidSymbolException(
            "Stock is not present in the portfolio."
        );
    }

    return holdings.at(index);
}

int Portfolio::getHoldingCount() const {
    return holdings.size();
}

int Portfolio::getTotalShareCount() const {
    int total = 0;

    for (int i = 0; i < holdings.size(); ++i) {
        total += holdings.at(i).getQuantity();
    }

    return total;
}

Holding* Portfolio::findHolding(
    const std::string& symbol
) {
    int index = findHoldingIndex(symbol);

    return index < 0 ? 0 : &holdings.at(index);
}

const Holding* Portfolio::findHolding(
    const std::string& symbol
) const {
    int index = findHoldingIndex(symbol);

    return index < 0 ? 0 : &holdings.at(index);
}

Holding& Portfolio::getHoldingAt(int index) {
    return holdings.at(index);
}

const Holding& Portfolio::getHoldingAt(int index) const {
    return holdings.at(index);
}

void Portfolio::setStockPrice(
    const std::string& symbol,
    const core::Money& price
) {
    getOrCreateHolding(symbol).setLastPrice(price);
}

core::Money Portfolio::getTotalCost() const {
    long long total = 0;

    for (int i = 0; i < holdings.size(); ++i) {
        total += holdings.at(i).getTotalCost().getPaise();
    }

    return core::Money::fromPaise(total);
}

core::Money Portfolio::getTotalMarketValue() const {
    long long total = 0;

    for (int i = 0; i < holdings.size(); ++i) {
        total += holdings.at(i).getMarketValue().getPaise();
    }

    return core::Money::fromPaise(total);
}

core::Money Portfolio::getTotalRealisedProfitLoss() const {
    long long total = 0;

    for (int i = 0; i < holdings.size(); ++i) {
        total +=
            holdings.at(i).getRealisedProfitLoss().getPaise();
    }

    return core::Money::fromPaise(total);
}

core::Money Portfolio::getTotalUnrealisedProfitLoss() const {
    long long total = 0;

    for (int i = 0; i < holdings.size(); ++i) {
        total +=
            holdings.at(i).getUnrealisedProfitLoss().getPaise();
    }

    return core::Money::fromPaise(total);
}

core::Money Portfolio::getTotalDividendIncome() const {
    long long total = 0;

    for (int i = 0; i < holdings.size(); ++i) {
        total +=
            holdings.at(i).getDividendIncome().getPaise();
    }

    return core::Money::fromPaise(total);
}

core::Money Portfolio::getTotalReturn() const {
    return core::Money::fromPaise(
        getTotalRealisedProfitLoss().getPaise() +
        getTotalUnrealisedProfitLoss().getPaise() +
        getTotalDividendIncome().getPaise()
    );
}

void Portfolio::applyTransaction(
    const transactions::Transaction& transaction
) {
    transaction.apply(*this);
}

void Portfolio::recordBuy(
    const std::string& symbol,
    int quantity,
    const core::Money& price,
    const core::Money& fees,
    const core::Date& date,
    long transactionId
) {
    getOrCreateHolding(symbol).addBuy(
        transactionId,
        date,
        quantity,
        price,
        fees
    );
}

void Portfolio::recordSell(
    const std::string& symbol,
    int quantity,
    const core::Money& price,
    const core::Money& fees,
    const core::Date& date,
    long transactionId
) {
    (void)date;
    (void)transactionId;

    requireHolding(symbol).addSell(
        quantity,
        price,
        fees
    );
}

void Portfolio::recordDividend(
    const std::string& symbol,
    int quantity,
    const core::Money& amountPerShare,
    const core::Date& date,
    long transactionId
) {
    (void)date;
    (void)transactionId;

    requireHolding(symbol).addDividend(
        quantity,
        amountPerShare
    );
}

void Portfolio::recordSplit(
    const std::string& symbol,
    int numerator,
    int denominator,
    const core::Date& date,
    long transactionId
) {
    (void)date;
    (void)transactionId;

    requireHolding(symbol).applySplit(
        numerator,
        denominator
    );
}

void Portfolio::recordBonus(
    const std::string& symbol,
    int numerator,
    int denominator,
    const core::Date& date,
    long transactionId
) {
    (void)date;
    (void)transactionId;

    requireHolding(symbol).applyBonus(
        numerator,
        denominator
    );
}

}
}