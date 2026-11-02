#ifndef PORTFOLIO_HOLDING_H
#define PORTFOLIO_HOLDING_H

#include <string>

#include "portfolio/core/Date.h"
#include "portfolio/core/Money.h"
#include "portfolio/core/Exceptions.h"
#include "portfolio/templates/DynamicArray.h"
#include "portfolio/portfolio/Lot.h"

namespace portfolio {
namespace portfolio_data {

class Holding {
private:
    std::string symbol;
    templates::DynamicArray<Lot> lots;

    core::Money realisedProfitLoss;
    core::Money dividendIncome;
    core::Money lastPrice;

    long nextLotId;

public:
    Holding()
        : symbol(""), lots(), realisedProfitLoss(),
          dividendIncome(), lastPrice(), nextLotId(1) {}

    explicit Holding(const std::string& stockSymbol)
        : symbol(stockSymbol), lots(), realisedProfitLoss(),
          dividendIncome(), lastPrice(), nextLotId(1) {}

    std::string getSymbol() const {
        return symbol;
    }

    int getQuantity() const {
        int total = 0;

        for (int i = 0; i < lots.size(); ++i) {
            total += lots.at(i).getQuantity();
        }

        return total;
    }

    int getLotCount() const {
        return lots.size();
    }

    Lot getLot(int index) const {
        return lots.at(index);
    }

    core::Money getTotalCost() const {
        long long total = 0;

        for (int i = 0; i < lots.size(); ++i) {
            total += lots.at(i).getRemainingCost().getPaise();
        }

        return core::Money::fromPaise(total);
    }

    core::Money getAverageCostPerShare() const {
        int quantity = getQuantity();

        if (quantity == 0) {
            return core::Money();
        }

        return core::Money::fromPaise(
            getTotalCost().getPaise() / quantity
        );
    }

    core::Money getLastPrice() const {
        return lastPrice;
    }

    core::Money getMarketValue() const {
        return core::Money::fromPaise(
            lastPrice.getPaise() *
            static_cast<long long>(getQuantity())
        );
    }

    core::Money getUnrealisedProfitLoss() const {
        return core::Money::fromPaise(
            getMarketValue().getPaise() -
            getTotalCost().getPaise()
        );
    }

    core::Money getRealisedProfitLoss() const {
        return realisedProfitLoss;
    }

    core::Money getDividendIncome() const {
        return dividendIncome;
    }

    void setLastPrice(const core::Money& price) {
        if (price.getPaise() < 0) {
            throw ::portfolio::InvalidTransactionException(
                "Stock price cannot be negative."
            );
        }

        lastPrice = price;
    }

    void addBuy(long transactionId, const core::Date& date,
                int quantity, const core::Money& price,
                const core::Money& fees) {

        if (quantity <= 0 || price.getPaise() < 0 ||
            fees.getPaise() < 0) {
            throw ::portfolio::InvalidTransactionException(
                "Invalid buy transaction."
            );
        }

        long long totalCost =
            price.getPaise() * static_cast<long long>(quantity)
            + fees.getPaise();

        lots.pushBack(
            Lot(
                nextLotId++,
                transactionId,
                date,
                quantity,
                core::Money::fromPaise(totalCost)
            )
        );
    }

    void addSell(int quantity, const core::Money& salePrice,
                 const core::Money& fees) {

        if (quantity <= 0 || salePrice.getPaise() < 0 ||
            fees.getPaise() < 0) {
            throw ::portfolio::InvalidTransactionException(
                "Invalid sell transaction."
            );
        }

        if (quantity > getQuantity()) {
            throw ::portfolio::InsufficientSharesException(
                "You do not own enough shares to complete this sale."
            );
        }

        long long costSold = 0;
        int remaining = quantity;

        // FIFO: oldest purchase lots are consumed first.
        while (remaining > 0) {
            int index = 0;

            while (index < lots.size() &&
                   lots.at(index).isEmpty()) {
                ++index;
            }

            if (index == lots.size()) {
                throw ::portfolio::InsufficientSharesException(
                    "Unable to find enough shares in purchase lots."
                );
            }

            int available = lots.at(index).getQuantity();

            int take = available < remaining
                ? available
                : remaining;

            costSold +=
                lots.at(index).removeShares(take).getPaise();

            remaining -= take;
        }

        long long proceeds =
            salePrice.getPaise() *
            static_cast<long long>(quantity) -
            fees.getPaise();

        realisedProfitLoss = core::Money::fromPaise(
            realisedProfitLoss.getPaise() +
            proceeds - costSold
        );

        for (int i = lots.size() - 1; i >= 0; --i) {
            if (lots.at(i).isEmpty()) {
                lots.removeAt(i);
            }
        }
    }

    void addDividend(int eligibleShares,
                     const core::Money& amountPerShare) {

        if (eligibleShares < 0 ||
            amountPerShare.getPaise() < 0) {
            throw ::portfolio::InvalidTransactionException(
                "Invalid dividend transaction."
            );
        }

        long long income =
            static_cast<long long>(eligibleShares) *
            amountPerShare.getPaise();

        dividendIncome = core::Money::fromPaise(
            dividendIncome.getPaise() + income
        );
    }

    void applySplit(int numerator, int denominator) {
        if (numerator <= 0 || denominator <= 0) {
            throw ::portfolio::InvalidTransactionException(
                "Invalid stock split ratio."
            );
        }

        // Validate all lots before modifying any of them.
        for (int i = 0; i < lots.size(); ++i) {
            long long scaled =
                static_cast<long long>(
                    lots.at(i).getQuantity()
                ) * numerator;

            if (scaled % denominator != 0) {
                throw ::portfolio::InvalidTransactionException(
                    "This split creates fractional shares; "
                    "fractional shares are unsupported."
                );
            }

            if (scaled / denominator > 2147483647LL) {
                throw ::portfolio::InvalidTransactionException(
                    "Share quantity is too large."
                );
            }
        }

        for (int i = 0; i < lots.size(); ++i) {
            int updated = static_cast<int>(
                (static_cast<long long>(
                    lots.at(i).getQuantity()
                ) * numerator) / denominator
            );

            lots.at(i).setQuantity(updated);
        }

        for (int i = lots.size() - 1; i >= 0; --i) {
            if (lots.at(i).isEmpty()) {
                lots.removeAt(i);
            }
        }
    }

    void applyBonus(int numerator, int denominator) {
        if (numerator <= 0 || denominator <= 0) {
            throw ::portfolio::InvalidTransactionException(
                "Invalid bonus issue ratio."
            );
        }

        // Validate all lots before modifying any of them.
        for (int i = 0; i < lots.size(); ++i) {
            long long bonus =
                static_cast<long long>(
                    lots.at(i).getQuantity()
                ) * numerator;

            if (bonus % denominator != 0) {
                throw ::portfolio::InvalidTransactionException(
                    "This bonus issue creates fractional shares; "
                    "fractional shares are unsupported."
                );
            }

            if (bonus / denominator >
                2147483647LL - lots.at(i).getQuantity()) {
                throw ::portfolio::InvalidTransactionException(
                    "Share quantity is too large."
                );
            }
        }

        for (int i = 0; i < lots.size(); ++i) {
            int bonus = static_cast<int>(
                (static_cast<long long>(
                    lots.at(i).getQuantity()
                ) * numerator) / denominator
            );

            lots.at(i).increaseQuantity(bonus);
        }
    }
};

}
}

#endif