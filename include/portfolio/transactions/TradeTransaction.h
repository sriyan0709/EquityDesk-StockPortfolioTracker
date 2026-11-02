#ifndef PORTFOLIO_TRADE_TRANSACTION_H
#define PORTFOLIO_TRADE_TRANSACTION_H

#include "portfolio/transactions/Transaction.h"

namespace portfolio {
namespace transactions {

class TradeTransaction : public Transaction {
protected:
    int quantity;
    core::Money pricePerShare;
    core::Money fees;

public:
    TradeTransaction(
        const std::string& sym,
        int qty,
        const core::Money& price,
        const core::Money& transactionFees,
        const core::Date& date
    )
        : Transaction(sym, date),
          quantity(qty),
          pricePerShare(price),
          fees(transactionFees) {
    }

    int getQuantity() const {
        return quantity;
    }

    core::Money getPricePerShare() const {
        return pricePerShare;
    }

    core::Money getFees() const {
        return fees;
    }

    core::Money getGrossAmount() const {
        return core::Money::fromPaise(
            static_cast<long long>(quantity) *
            pricePerShare.getPaise()
        );
    }
};

}
}

#endif