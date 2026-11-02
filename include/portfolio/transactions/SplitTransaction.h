#ifndef PORTFOLIO_SPLIT_TRANSACTION_H
#define PORTFOLIO_SPLIT_TRANSACTION_H

#include "portfolio/transactions/CorporateAction.h"

namespace portfolio {
namespace transactions {

class SplitTransaction : public CorporateAction {
private:
    int ratioNumerator;
    int ratioDenominator;

public:
    SplitTransaction(
        const std::string& sym,
        int numerator,
        int denominator,
        const core::Date& date
    )
        : core::Entity(0, date),
          CorporateAction(sym, date),
          ratioNumerator(numerator),
          ratioDenominator(denominator) {

        if (numerator <= 0 || denominator <= 0) {
            throw InvalidTransactionException(
                "Split ratio must contain positive values."
            );
        }
    }

    int getNumerator() const {
        return ratioNumerator;
    }

    int getDenominator() const {
        return ratioDenominator;
    }

    TxnType getType() const {
        return SPLIT;
    }

    void apply(TransactionActions& actions) const {
        actions.recordSplit(
            symbol,
            ratioNumerator,
            ratioDenominator,
            transactionDate,
            getId()
        );
    }

    Transaction* clone() const {
        return new SplitTransaction(*this);
    }

    std::string describe() const {
        return "SPLIT " + symbol;
    }
};

}
}

#endif