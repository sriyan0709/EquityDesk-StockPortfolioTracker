#ifndef PORTFOLIO_SELL_TRANSACTION_H
#define PORTFOLIO_SELL_TRANSACTION_H

#include "portfolio/transactions/TradeTransaction.h"

namespace portfolio {
namespace transactions {

class SellTransaction : public TradeTransaction {
public:
    SellTransaction(
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
        return SELL;
    }

    void apply(TransactionActions& actions) const {
        actions.recordSell(
            symbol,
            quantity,
            pricePerShare,
            fees,
            transactionDate,
            getId()
        );
    }

    Transaction* clone() const {
        return new SellTransaction(*this);
    }

    std::string describe() const {
        return "SELL " + symbol;
    }
};

}
}

#endif