#ifndef PORTFOLIO_CORPORATE_ACTION_H
#define PORTFOLIO_CORPORATE_ACTION_H

#include "portfolio/transactions/Transaction.h"
#include "portfolio/core/Exceptions.h"
namespace portfolio {
namespace transactions {

class CorporateAction : public Transaction {
public:
    CorporateAction(
        const std::string& sym,
        const core::Date& date
    )
        : Transaction(sym, date) {
    }

    virtual ~CorporateAction() {
    }
};

}
}

#endif