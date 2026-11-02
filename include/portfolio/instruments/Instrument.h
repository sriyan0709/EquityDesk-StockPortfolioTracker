#ifndef PORTFOLIO_INSTRUMENT_H
#define PORTFOLIO_INSTRUMENT_H

#include <string>

namespace portfolio {
namespace instruments {

class Instrument {
protected:
    std::string symbol;
    std::string companyName;
    std::string exchange;

public:
    Instrument(
        const std::string& sym,
        const std::string& name,
        const std::string& exch
    )
        : symbol(sym), companyName(name), exchange(exch) {
    }

    virtual ~Instrument() {
    }

    std::string getSymbol() const {
        return symbol;
    }

    std::string getCompanyName() const {
        return companyName;
    }

    std::string getExchange() const {
        return exchange;
    }

    virtual std::string getInstrumentType() const = 0;

    virtual std::string describe() const = 0;
};

class Stock : public Instrument {
public:
    Stock(
        const std::string& sym,
        const std::string& name,
        const std::string& exch
    )
        : Instrument(sym, name, exch) {
    }

    std::string getInstrumentType() const {
        return "Equity Stock";
    }

    std::string describe() const {
        return companyName + " (" + symbol + ")";
    }
};

class Etf : public Instrument {
public:
    Etf(
        const std::string& sym,
        const std::string& name,
        const std::string& exch
    )
        : Instrument(sym, name, exch) {
    }

    std::string getInstrumentType() const {
        return "Exchange Traded Fund";
    }

    std::string describe() const {
        return companyName + " (" + symbol + ")";
    }
};

}
}

#endif