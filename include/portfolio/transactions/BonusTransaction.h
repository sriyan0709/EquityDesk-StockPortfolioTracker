#ifndef PORTFOLIO_BONUS_TRANSACTION_H
#define PORTFOLIO_BONUS_TRANSACTION_H

#include "portfolio/transactions/CorporateAction.h"

namespace portfolio {
namespace transactions {

class BonusTransaction : public CorporateAction {
private:
    int ratioNumerator;
    int ratioDenominator;

public:
    BonusTransaction(
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
                "Bonus ratio must contain positive values."
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
        return BONUS;
    }

    void apply(TransactionActions& actions) const {
        actions.recordBonus(
            symbol,
            ratioNumerator,
            ratioDenominator,
            transactionDate,
            getId()
        );
    }

    Transaction* clone() const {
        return new BonusTransaction(*this);
    }

    std::string describe() const {
        return "BONUS " + symbol;
    }
};

}
}

#endif