#ifndef PORTFOLIO_TRANSACTION_RECORD_H
#define PORTFOLIO_TRANSACTION_RECORD_H

#include <string>

#include "portfolio/core/Date.h"
#include "portfolio/core/Money.h"
#include "portfolio/transactions/Transaction.h"

namespace portfolio {
namespace storage {

struct TransactionRecord {
    long transactionId;
    transactions::TxnType type;
    std::string symbol;
    core::Date date;

    int quantity;
    long long pricePaise;
    long long feesPaise;

    int numerator;
    int denominator;

    TransactionRecord()
        : transactionId(0),
          type(transactions::BUY),
          symbol(""),
          date(),
          quantity(0),
          pricePaise(0),
          feesPaise(0),
          numerator(0),
          denominator(0) {
    }
};

}
}

#endif