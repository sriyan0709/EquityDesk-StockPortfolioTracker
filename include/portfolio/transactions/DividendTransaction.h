#ifndef PORTFOLIO_DIVIDEND_TRANSACTION_H
#define PORTFOLIO_DIVIDEND_TRANSACTION_H

#include "portfolio/transactions/CorporateAction.h"

namespace portfolio {
namespace transactions {

class DividendTransaction : public CorporateAction {
private:
    int eligibleShares;
    core::Money amountPerShare;

public:
    DividendTransaction(
        const std::string& sym,
        int shares,
        const core::Money& dividendPerShare,
        const core::Date& date
    )
        : core::Entity(0, date),
          CorporateAction(sym, date),
          eligibleShares(shares),
          amountPerShare(dividendPerShare) {
    }

    int getEligibleShares() const {
        return eligibleShares;
    }

    core::Money getAmountPerShare() const {
        return amountPerShare;
    }

    TxnType getType() const {
        return DIVIDEND;
    }

    void apply(TransactionActions& actions) const {
        actions.recordDividend(
            symbol,
            eligibleShares,
            amountPerShare,
            transactionDate,
            getId()
        );
    }

    Transaction* clone() const {
        return new DividendTransaction(*this);
    }

    std::string describe() const {
        return "DIVIDEND " + symbol;
    }
};

}
}

#endif