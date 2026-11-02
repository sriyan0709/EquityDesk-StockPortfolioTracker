#ifndef PORTFOLIO_LOT_H
#define PORTFOLIO_LOT_H

#include "portfolio/core/Date.h"
#include "portfolio/core/Money.h"
#include "portfolio/core/Exceptions.h"

namespace portfolio {
namespace portfolio_data {

class Lot {
private:
    long lotId;
    long transactionId;
    core::Date purchaseDate;
    int quantity;
    long long remainingCostPaise;

public:
    Lot()
        : lotId(0), transactionId(0), purchaseDate(),
          quantity(0), remainingCostPaise(0) {}

    Lot(long id, long txnId, const core::Date& date,
        int shares, const core::Money& totalCost)
        : lotId(id), transactionId(txnId), purchaseDate(date),
          quantity(shares), remainingCostPaise(totalCost.getPaise()) {
        if (shares < 0 || totalCost.getPaise() < 0) {
            throw ::portfolio::InvalidTransactionException(
                "Lot quantity and cost cannot be negative."
            );
        }
    }

    long getLotId() const { return lotId; }
    long getTransactionId() const { return transactionId; }
    core::Date getPurchaseDate() const { return purchaseDate; }
    int getQuantity() const { return quantity; }

    core::Money getRemainingCost() const {
        return core::Money::fromPaise(remainingCostPaise);
    }

    bool isEmpty() const { return quantity == 0; }

    core::Money removeShares(int shares) {
        if (shares <= 0 || shares > quantity) {
            throw ::portfolio::InvalidTransactionException(
                "Invalid quantity while removing shares from a lot."
            );
        }

        long long costRemoved = (shares == quantity)
            ? remainingCostPaise
            : (remainingCostPaise * shares) / quantity;

        quantity -= shares;
        remainingCostPaise -= costRemoved;

        return core::Money::fromPaise(costRemoved);
    }

    void increaseQuantity(int additionalShares) {
        if (additionalShares < 0 ||
            additionalShares > 2147483647 - quantity) {
            throw ::portfolio::InvalidTransactionException(
                "Invalid additional share quantity."
            );
        }

        quantity += additionalShares;
    }

    void setQuantity(int newQuantity) {
        if (newQuantity < 0) {
            throw ::portfolio::InvalidTransactionException(
                "Lot quantity cannot be negative."
            );
        }

        quantity = newQuantity;

        if (quantity == 0) {
            remainingCostPaise = 0;
        }
    }
};

}
}

#endif