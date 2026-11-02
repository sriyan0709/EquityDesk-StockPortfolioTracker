#ifndef PORTFOLIO_TRANSACTION_H
#define PORTFOLIO_TRANSACTION_H

#include "portfolio/core/Entity.h"
#include "portfolio/core/Money.h"
#include "portfolio/core/Date.h"

#include <string>

namespace portfolio {
namespace transactions {

enum TxnType {
    BUY,
    SELL,
    DIVIDEND,
    SPLIT,
    BONUS
};

enum ReasonTag {
    NONE,
    NEWS,
    VALUATION,
    TIP,
    TECHNICAL,
    LONG_TERM,
    FOMO
};

class TransactionActions {
public:
    virtual void recordBuy(
        const std::string& symbol,
        int quantity,
        const core::Money& price,
        const core::Money& fees,
        const core::Date& date,
        long transactionId
    ) = 0;

    virtual void recordSell(
        const std::string& symbol,
        int quantity,
        const core::Money& price,
        const core::Money& fees,
        const core::Date& date,
        long transactionId
    ) = 0;

    virtual void recordDividend(
        const std::string& symbol,
        int quantity,
        const core::Money& amountPerShare,
        const core::Date& date,
        long transactionId
    ) = 0;

    virtual void recordSplit(
        const std::string& symbol,
        int numerator,
        int denominator,
        const core::Date& date,
        long transactionId
    ) = 0;

    virtual void recordBonus(
        const std::string& symbol,
        int numerator,
        int denominator,
        const core::Date& date,
        long transactionId
    ) = 0;

    virtual ~TransactionActions() {
    }
};

class Transaction : public virtual core::Entity {
protected:
    std::string symbol;
    core::Date transactionDate;

    static long nextId;
    static int transactionCount;

public:
    Transaction(
        const std::string& sym,
        const core::Date& date
    )
        : symbol(sym), transactionDate(date) {

        setEntity(nextId++, date);
        ++transactionCount;
    }

    Transaction(const Transaction& other)
        : core::Entity(other.getId(), other.getCreatedDate()),
          symbol(other.symbol),
          transactionDate(other.transactionDate) {

        ++transactionCount;
    }

    virtual ~Transaction() {
        --transactionCount;
    }

    std::string getSymbol() const {
        return symbol;
    }

    core::Date getTransactionDate() const {
        return transactionDate;
    }

    static int count() {
        return transactionCount;
    }

    static void reserveNextId(long usedId) {
        if (nextId <= usedId) {
            nextId = usedId + 1;
        }
    }

    virtual TxnType getType() const = 0;

    virtual void apply(TransactionActions& actions) const = 0;

    virtual Transaction* clone() const = 0;

    virtual std::string describe() const = 0;
};

}
}

#endif