#ifndef PORTFOLIO_MONEY_H
#define PORTFOLIO_MONEY_H

#include <iostream>
#include <string>

namespace portfolio {
namespace core {

class Money {
private:
    long long paise;

public:
    Money(long long rupees = 0, long long extraPaise = 0);

    static Money fromPaise(long long value);

    long long getPaise() const;
    long long getRupees() const;
    int getRemainingPaise() const;

    std::string toString() const;

    Money operator+(const Money& other) const;
    Money operator-(const Money& other) const;
    Money operator-() const;

    Money& operator+=(const Money& other);
    Money& operator-=(const Money& other);

    bool operator==(const Money& other) const;
    bool operator!=(const Money& other) const;
    bool operator<(const Money& other) const;
    bool operator>(const Money& other) const;
    bool operator<=(const Money& other) const;
    bool operator>=(const Money& other) const;

    friend std::ostream& operator<<(std::ostream& out,
                                    const Money& money);

    friend std::istream& operator>>(std::istream& in,
                                    Money& money);
};

}
}

#endif