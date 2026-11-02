#ifndef PORTFOLIO_ANNOTATED_TRADE_H
#define PORTFOLIO_ANNOTATED_TRADE_H
#include "portfolio/core/Exceptions.h"
#include "portfolio/transactions/TradeTransaction.h"
#include "portfolio/core/Exceptions.h"
namespace portfolio {
namespace transactions {

class JournalNote : public virtual core::Entity {
protected:
    ReasonTag reason;
    int confidence;
    std::string note;

public:
    JournalNote(
        ReasonTag tradeReason = NONE,
        int tradeConfidence = 0,
        const std::string& tradeNote = ""
    )
        : reason(tradeReason),
          confidence(tradeConfidence),
          note(tradeNote) {

        if (confidence < 0 || confidence > 5) {
            throw InvalidTransactionException(
                "Confidence must be between 0 and 5."
            );
        }
    }

    virtual ~JournalNote() {
    }

    ReasonTag getReason() const {
        return reason;
    }

    int getConfidence() const {
        return confidence;
    }

    std::string getNote() const {
        return note;
    }

    virtual std::string describe() const {
        return note;
    }
};

class AnnotatedTrade :
    public TradeTransaction,
    public JournalNote {

private:
    bool buyTrade;

public:
    AnnotatedTrade(
        const std::string& sym,
        int qty,
        const core::Money& price,
        const core::Money& fees,
        const core::Date& date,
        bool isBuy,
        ReasonTag tradeReason,
        int tradeConfidence,
        const std::string& tradeNote
    )
        : core::Entity(0, date),
          TradeTransaction(sym, qty, price, fees, date),
          JournalNote(tradeReason, tradeConfidence, tradeNote),
          buyTrade(isBuy) {
    }

    TxnType getType() const {
        return buyTrade ? BUY : SELL;
    }

    void apply(TransactionActions& actions) const {
        if (buyTrade) {
            actions.recordBuy(
                symbol,
                quantity,
                pricePerShare,
                fees,
                transactionDate,
                getId()
            );
        }
        else {
            actions.recordSell(
                symbol,
                quantity,
                pricePerShare,
                fees,
                transactionDate,
                getId()
            );
        }
    }

    Transaction* clone() const {
        return new AnnotatedTrade(*this);
    }

    std::string describe() const {
        return std::string(buyTrade ? "ANNOTATED BUY " : "ANNOTATED SELL ")
            + symbol + " | " + note;
    }

    bool isBuyTrade() const {
        return buyTrade;
    }
};

inline std::string reasonToString(ReasonTag reason) {
    switch (reason) {
        case NEWS:      return "News";
        case VALUATION: return "Valuation";
        case TIP:       return "Tip";
        case TECHNICAL: return "Technical";
        case LONG_TERM: return "Long-term";
        case FOMO:      return "FOMO";
        default:        return "None";
    }
}

}
}

#endif