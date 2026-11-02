#ifndef PORTFOLIO_BUY_TRANSACTION_H
#define PORTFOLIO_BUY_TRANSACTION_H

#include "portfolio/transactions/TradeTransaction.h"

namespace portfolio {
namespace transactions {

class BuyTransaction : public TradeTransaction {
public:
    BuyTransaction(
        const std::string& sym,
        int qty,
        const core::Money& price,
        const core::Money& fees,
        const core::Date& date
    )
        : core::Entity(0, date),
          TradeTransaction(sym, qty, price, fees, date) {
    }

    TxnType getType() const {
        return BUY;
    }

    void apply(TransactionActions& actions) const {
        actions.recordBuy(
            symbol,
            quantity,
            pricePerShare,
            fees,
            transactionDate,
            getId()
        );
    }

    Transaction* clone() const {
        return new BuyTransaction(*this);
    }

    std::string describe() const {
        return "BUY " + symbol;
    }
};

}
}

#endif