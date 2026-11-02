#include "portfolio/storage/TransactionStore.h"

#include <fstream>
#include <sstream>

#include "portfolio/core/Exceptions.h"

#include "portfolio/transactions/BuyTransaction.h"
#include "portfolio/transactions/SellTransaction.h"
#include "portfolio/transactions/DividendTransaction.h"
#include "portfolio/transactions/SplitTransaction.h"
#include "portfolio/transactions/BonusTransaction.h"

namespace portfolio {
namespace storage {

TransactionStore::TransactionStore()
    : records() {
}

int TransactionStore::size() const {
    return records.size();
}

bool TransactionStore::isEmpty() const {
    return records.isEmpty();
}

const TransactionRecord& TransactionStore::get(int index) const {
    return records.at(index);
}

bool TransactionStore::isValidSymbol(
    const std::string& symbol
) {
    if (symbol.empty() || symbol.size() > 30) {
        return false;
    }

    for (std::size_t i = 0; i < symbol.size(); ++i) {
        char ch = symbol[i];

        if (!((ch >= 'A' && ch <= 'Z') ||
              (ch >= 'a' && ch <= 'z') ||
              (ch >= '0' && ch <= '9') ||
              ch == '.' || ch == '-' || ch == '_')) {
            return false;
        }
    }

    return true;
}

void TransactionStore::add(
    const transactions::Transaction& transaction
) {
    TransactionRecord record;

    record.transactionId = transaction.getId();
    record.type = transaction.getType();
    record.symbol = transaction.getSymbol();
    record.date = transaction.getTransactionDate();

    if (!isValidSymbol(record.symbol)) {
        throw ::portfolio::InvalidSymbolException(
            "Invalid stock symbol in transaction."
        );
    }

    switch (record.type) {

        case transactions::BUY: {
            const transactions::BuyTransaction* p =
                dynamic_cast<const transactions::BuyTransaction*>(
                    &transaction
                );

            if (!p) {
                throw ::portfolio::InvalidTransactionException(
                    "Transaction type does not match BUY."
                );
            }

            record.quantity = p->getQuantity();
            record.pricePaise =
                p->getPricePerShare().getPaise();
            record.feesPaise =
                p->getFees().getPaise();

            break;
        }

        case transactions::SELL: {
            const transactions::SellTransaction* p =
                dynamic_cast<const transactions::SellTransaction*>(
                    &transaction
                );

            if (!p) {
                throw ::portfolio::InvalidTransactionException(
                    "Transaction type does not match SELL."
                );
            }

            record.quantity = p->getQuantity();
            record.pricePaise =
                p->getPricePerShare().getPaise();
            record.feesPaise =
                p->getFees().getPaise();

            break;
        }

        case transactions::DIVIDEND: {
            const transactions::DividendTransaction* p =
                dynamic_cast<const transactions::DividendTransaction*>(
                    &transaction
                );

            if (!p) {
                throw ::portfolio::InvalidTransactionException(
                    "Transaction type does not match DIVIDEND."
                );
            }

            record.quantity = p->getEligibleShares();
            record.pricePaise =
                p->getAmountPerShare().getPaise();

            break;
        }

        case transactions::SPLIT: {
            const transactions::SplitTransaction* p =
                dynamic_cast<const transactions::SplitTransaction*>(
                    &transaction
                );

            if (!p) {
                throw ::portfolio::InvalidTransactionException(
                    "Transaction type does not match SPLIT."
                );
            }

            record.numerator = p->getNumerator();
            record.denominator = p->getDenominator();

            break;
        }

        case transactions::BONUS: {
            const transactions::BonusTransaction* p =
                dynamic_cast<const transactions::BonusTransaction*>(
                    &transaction
                );

            if (!p) {
                throw ::portfolio::InvalidTransactionException(
                    "Transaction type does not match BONUS."
                );
            }

            record.numerator = p->getNumerator();
            record.denominator = p->getDenominator();

            break;
        }

        default:
            throw ::portfolio::InvalidTransactionException(
                "Unknown transaction type."
            );
    }

    addRecord(record);
}

void TransactionStore::addRecord(
    const TransactionRecord& record
) {
    if (!isValidSymbol(record.symbol)) {
        throw ::portfolio::InvalidSymbolException(
            "Invalid stock symbol."
        );
    }

    if (!record.date.isValid()) {
        throw ::portfolio::InvalidDateException(
            "Invalid transaction date."
        );
    }

    if (record.transactionId <= 0) {
        throw ::portfolio::InvalidTransactionException(
            "Transaction ID must be positive."
        );
    }

    if (record.type == transactions::BUY ||
        record.type == transactions::SELL) {

        if (record.quantity <= 0 ||
            record.pricePaise < 0 ||
            record.feesPaise < 0) {

            throw ::portfolio::InvalidTransactionException(
                "Invalid trade record."
            );
        }

    } else if (record.type == transactions::DIVIDEND) {

        if (record.quantity < 0 ||
            record.pricePaise < 0) {

            throw ::portfolio::InvalidTransactionException(
                "Invalid dividend record."
            );
        }

    } else if (record.type == transactions::SPLIT ||
               record.type == transactions::BONUS) {

        if (record.numerator <= 0 ||
            record.denominator <= 0) {

            throw ::portfolio::InvalidTransactionException(
                "Invalid corporate action record."
            );
        }

    } else {
        throw ::portfolio::InvalidTransactionException(
            "Unknown transaction type."
        );
    }

    for (int i = 0; i < records.size(); ++i) {
        if (records.at(i).transactionId ==
            record.transactionId) {

            throw ::portfolio::InvalidTransactionException(
                "Duplicate transaction ID."
            );
        }
    }

    records.pushBack(record);
}

bool TransactionStore::saveToFile(
    const std::string& filename
) const {
    std::ofstream file(filename.c_str());

    if (!file.is_open()) {
        return false;
    }

    file << "SPT1\n";

    for (int i = 0; i < records.size(); ++i) {
        const TransactionRecord& r = records.at(i);

        file << r.transactionId << '|'
             << static_cast<int>(r.type) << '|'
             << r.symbol << '|'
             << r.date.toString() << '|'
             << r.quantity << '|'
             << r.pricePaise << '|'
             << r.feesPaise << '|'
             << r.numerator << '|'
             << r.denominator << '\n';

        if (!file.good()) {
            return false;
        }
    }

    file.close();

    return !file.fail();
}

bool TransactionStore::parseRecord(
    const std::string& line,
    TransactionRecord& r
) {
    std::istringstream input(line);

    std::string id;
    std::string type;
    std::string symbol;
    std::string date;
    std::string quantity;
    std::string price;
    std::string fees;
    std::string numerator;
    std::string denominator;

    if (!std::getline(input, id, '|') ||
        !std::getline(input, type, '|') ||
        !std::getline(input, symbol, '|') ||
        !std::getline(input, date, '|') ||
        !std::getline(input, quantity, '|') ||
        !std::getline(input, price, '|') ||
        !std::getline(input, fees, '|') ||
        !std::getline(input, numerator, '|') ||
        !std::getline(input, denominator)) {

        return false;
    }

    std::istringstream a(id);
    std::istringstream b(type);
    std::istringstream c(quantity);
    std::istringstream d(price);
    std::istringstream e(fees);
    std::istringstream f(numerator);
    std::istringstream g(denominator);
    std::istringstream h(date);

    int typeValue = -1;

    if (!(a >> r.transactionId) ||
        !(b >> typeValue) ||
        !(c >> r.quantity) ||
        !(d >> r.pricePaise) ||
        !(e >> r.feesPaise) ||
        !(f >> r.numerator) ||
        !(g >> r.denominator) ||
        !(h >> r.date)) {

        return false;
    }

    std::string extra;

    if ((a >> extra) ||
        (b >> extra) ||
        (c >> extra) ||
        (d >> extra) ||
        (e >> extra) ||
        (f >> extra) ||
        (g >> extra) ||
        (h >> extra)) {

        return false;
    }

    if (typeValue < transactions::BUY ||
        typeValue > transactions::BONUS) {

        return false;
    }

    r.type = static_cast<transactions::TxnType>(typeValue);
    r.symbol = symbol;

    return r.date.isValid() && isValidSymbol(r.symbol);
}

bool TransactionStore::loadFromFile(
    const std::string& filename
) {
    std::ifstream file(filename.c_str());

    if (!file.is_open()) {
        return false;
    }

    std::string header;

    if (!std::getline(file, header) ||
        header != "SPT1") {

        return false;
    }

    templates::DynamicArray<TransactionRecord> loaded;

    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        TransactionRecord record;

        if (!parseRecord(line, record)) {
            return false;
        }

        if (record.transactionId <= 0) {
            return false;
        }

        if (record.type == transactions::BUY ||
            record.type == transactions::SELL) {

            if (record.quantity <= 0 ||
                record.pricePaise < 0 ||
                record.feesPaise < 0) {

                return false;
            }

        } else if (record.type == transactions::DIVIDEND) {

            if (record.quantity < 0 ||
                record.pricePaise < 0) {

                return false;
            }

        } else if (record.numerator <= 0 ||
                   record.denominator <= 0) {

            return false;
        }

        for (int i = 0; i < loaded.size(); ++i) {
            if (loaded.at(i).transactionId ==
                record.transactionId) {

                return false;
            }
        }

        loaded.pushBack(record);
    }

    if (file.bad()) {
        return false;
    }

    records = loaded;

    return true;
}

transactions::Transaction*
TransactionStore::createTransaction(int index) const {

    const TransactionRecord& r = records.at(index);

    core::Money price =
        core::Money::fromPaise(r.pricePaise);

    core::Money fees =
        core::Money::fromPaise(r.feesPaise);

    switch (r.type) {

        case transactions::BUY:
            return new transactions::BuyTransaction(
                r.symbol,
                r.quantity,
                price,
                fees,
                r.date
            );

        case transactions::SELL:
            return new transactions::SellTransaction(
                r.symbol,
                r.quantity,
                price,
                fees,
                r.date
            );

        case transactions::DIVIDEND:
            return new transactions::DividendTransaction(
                r.symbol,
                r.quantity,
                price,
                r.date
            );

        case transactions::SPLIT:
            return new transactions::SplitTransaction(
                r.symbol,
                r.numerator,
                r.denominator,
                r.date
            );

        case transactions::BONUS:
            return new transactions::BonusTransaction(
                r.symbol,
                r.numerator,
                r.denominator,
                r.date
            );

        default:
            throw ::portfolio::InvalidTransactionException(
                "Unknown transaction type in storage."
            );
    }
}

void TransactionStore::replayInto(
    portfolio_data::Portfolio& destination
) const {

    templates::DynamicArray<TransactionRecord> ordered(records);

    // Stable insertion sort by transaction date.
    // Transactions on the same date retain their original order.

    for (int i = 1; i < ordered.size(); ++i) {

        TransactionRecord key = ordered.at(i);

        int j = i - 1;

        while (j >= 0 &&
               key.date < ordered.at(j).date) {

            ordered.at(j + 1) = ordered.at(j);

            --j;
        }

        ordered.at(j + 1) = key;
    }

    // Build a temporary portfolio first.
    // The destination is not modified if replay fails.

    portfolio_data::Portfolio rebuilt;

    for (int i = 0; i < ordered.size(); ++i) {

        const TransactionRecord& r = ordered.at(i);

        core::Money price =
            core::Money::fromPaise(r.pricePaise);

        core::Money fees =
            core::Money::fromPaise(r.feesPaise);

        switch (r.type) {

            case transactions::BUY:

                rebuilt.recordBuy(
                    r.symbol,
                    r.quantity,
                    price,
                    fees,
                    r.date,
                    r.transactionId
                );

                break;

            case transactions::SELL:

                rebuilt.recordSell(
                    r.symbol,
                    r.quantity,
                    price,
                    fees,
                    r.date,
                    r.transactionId
                );

                break;

            case transactions::DIVIDEND:

                rebuilt.recordDividend(
                    r.symbol,
                    r.quantity,
                    price,
                    r.date,
                    r.transactionId
                );

                break;

            case transactions::SPLIT:

                rebuilt.recordSplit(
                    r.symbol,
                    r.numerator,
                    r.denominator,
                    r.date,
                    r.transactionId
                );

                break;

            case transactions::BONUS:

                rebuilt.recordBonus(
                    r.symbol,
                    r.numerator,
                    r.denominator,
                    r.date,
                    r.transactionId
                );

                break;

            default:

                throw ::portfolio::InvalidTransactionException(
                    "Unknown transaction type during replay."
                );
        }
    }

    // Update the ID generator only after successful replay.

    for (int i = 0; i < ordered.size(); ++i) {

        transactions::Transaction::reserveNextId(
            ordered.at(i).transactionId
        );
    }

    destination = rebuilt;
}

void TransactionStore::clear() {
    records.clear();
}

}
}