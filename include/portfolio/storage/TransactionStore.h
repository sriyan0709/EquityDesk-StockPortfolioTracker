#ifndef PORTFOLIO_TRANSACTION_STORE_H
#define PORTFOLIO_TRANSACTION_STORE_H

#include <string>

#include "portfolio/templates/DynamicArray.h"
#include "portfolio/storage/TransactionRecord.h"
#include "portfolio/transactions/Transaction.h"
#include "portfolio/portfolio/Portfolio.h"

namespace portfolio {
namespace storage {

class TransactionStore {
private:
    templates::DynamicArray<TransactionRecord> records;

    static bool isValidSymbol(const std::string& symbol);

    static bool parseRecord(
        const std::string& line,
        TransactionRecord& record
    );

public:
    TransactionStore();

    int size() const;
    bool isEmpty() const;

    const TransactionRecord& get(int index) const;

    void add(const transactions::Transaction& transaction);

    void addRecord(const TransactionRecord& record);

    bool saveToFile(const std::string& filename) const;

    bool loadFromFile(const std::string& filename);

    transactions::Transaction* createTransaction(int index) const;

    void replayInto(
        portfolio_data::Portfolio& destination
    ) const;

    void clear();
};

}
}

#endif