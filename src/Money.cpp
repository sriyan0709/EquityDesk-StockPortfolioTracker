#include "portfolio/core/Money.h"

#include <sstream>
#include <iomanip>

namespace portfolio {
namespace core {

Money::Money(long long rupees, long long extraPaise) {
    paise = rupees * 100 + extraPaise;
}

Money Money::fromPaise(long long value) {
    Money result;
    result.paise = value;
    return result;
}

long long Money::getPaise() const {
    return paise;
}

long long Money::getRupees() const {
    return paise / 100;
}

int Money::getRemainingPaise() const {
    long long remainder = paise % 100;

    if (remainder < 0) {
        remainder = -remainder;
    }

    return static_cast<int>(remainder);
}

std::string Money::toString() const {
    std::ostringstream output;

    bool negative = paise < 0;

    unsigned long long absoluteValue =
        negative
        ? static_cast<unsigned long long>(-(paise + 1)) + 1
        : static_cast<unsigned long long>(paise);

    unsigned long long rupees = absoluteValue / 100;
    unsigned int cents = static_cast<unsigned int>(absoluteValue % 100);

    std::string digits;
    std::ostringstream number;

    number << rupees;
    digits = number.str();

    std::string grouped;

    if (digits.length() <= 3) {
        grouped = digits;
    }
    else {
        grouped = digits.substr(digits.length() - 3);

        int position = static_cast<int>(digits.length()) - 4;

        while (position >= 0) {
            int start = position - 1;

            if (start < 0) {
                start = 0;
            }

            grouped = digits.substr(
                static_cast<std::string::size_type>(start),
                static_cast<std::string::size_type>(position - start + 1)
            ) + "," + grouped;

            position -= 2;
        }
    }

    if (negative) {
        grouped = "-" + grouped;
    }

    output << "Rs. " << grouped
           << "." << std::setfill('0')
           << std::setw(2) << cents;

    return output.str();
}

Money Money::operator+(const Money& other) const {
    return fromPaise(paise + other.paise);
}

Money Money::operator-(const Money& other) const {
    return fromPaise(paise - other.paise);
}

Money Money::operator-() const {
    return fromPaise(-paise);
}

Money& Money::operator+=(const Money& other) {
    paise += other.paise;
    return *this;
}

Money& Money::operator-=(const Money& other) {
    paise -= other.paise;
    return *this;
}

bool Money::operator==(const Money& other) const {
    return paise == other.paise;
}

bool Money::operator!=(const Money& other) const {
    return paise != other.paise;
}

bool Money::operator<(const Money& other) const {
    return paise < other.paise;
}

bool Money::operator>(const Money& other) const {
    return paise > other.paise;
}

bool Money::operator<=(const Money& other) const {
    return paise <= other.paise;
}

bool Money::operator>=(const Money& other) const {
    return paise >= other.paise;
}

std::ostream& operator<<(std::ostream& out,
                         const Money& money) {
    out << money.toString();
    return out;
}

std::istream& operator>>(std::istream& in,
                         Money& money) {
    long long rupees;
    int cents;

    in >> rupees;

    if (!in) {
        return in;
    }

    char separator;

    if (in.peek() == '.') {
        in >> separator >> cents;

        if (cents < 0 || cents > 99) {
            in.setstate(std::ios::failbit);
            return in;
        }

        money = Money(rupees, cents);
    }
    else {
        money = Money(rupees);
    }

    return in;
}

}
}