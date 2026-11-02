#include "portfolio/core/Date.h"
#include "portfolio/core/Exceptions.h"

#include <sstream>
#include <iomanip>

namespace portfolio {
namespace core {

Date::Date()
    : day(1), month(1), year(2000) {
}

Date::Date(int d, int m, int y)
    : day(d), month(m), year(y) {

    if (!isValid()) {
        throw InvalidDateException("Invalid date supplied.");
    }
}

bool Date::isLeapYear(int y) {
    return (y % 400 == 0) ||
           ((y % 4 == 0) && (y % 100 != 0));
}

int Date::daysInMonth(int m, int y) {
    const int monthDays[12] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (m == 2 && isLeapYear(y)) {
        return 29;
    }

    if (m < 1 || m > 12) {
        return 0;
    }

    return monthDays[m - 1];
}

int Date::getDay() const {
    return day;
}

int Date::getMonth() const {
    return month;
}

int Date::getYear() const {
    return year;
}

bool Date::isValid() const {
    if (year < 1900 || year > 9999) {
        return false;
    }

    if (month < 1 || month > 12) {
        return false;
    }

    return day >= 1 && day <= daysInMonth(month, year);
}

long long Date::serialDay() const {
    long long total = 0;

    int previousYears = year - 1;

    total = static_cast<long long>(previousYears) * 365;
    total += previousYears / 4;
    total -= previousYears / 100;
    total += previousYears / 400;

    for (int m = 1; m < month; ++m) {
        total += daysInMonth(m, year);
    }

    total += day;

    return total;
}

std::string Date::toString() const {
    std::ostringstream output;

    output << std::setfill('0')
           << std::setw(4) << year << "-"
           << std::setw(2) << month << "-"
           << std::setw(2) << day;

    return output.str();
}

bool Date::operator<(const Date& other) const {
    return serialDay() < other.serialDay();
}

bool Date::operator>(const Date& other) const {
    return other < *this;
}

bool Date::operator<=(const Date& other) const {
    return !(*this > other);
}

bool Date::operator>=(const Date& other) const {
    return !(*this < other);
}

bool Date::operator==(const Date& other) const {
    return day == other.day &&
           month == other.month &&
           year == other.year;
}

bool Date::operator!=(const Date& other) const {
    return !(*this == other);
}

Date& Date::operator++() {
    ++day;

    if (day > daysInMonth(month, year)) {
        day = 1;
        ++month;

        if (month > 12) {
            month = 1;
            ++year;
        }
    }

    if (year > 9999) {
        throw InvalidDateException("Date exceeded supported range.");
    }

    return *this;
}

std::ostream& operator<<(std::ostream& out,
                         const Date& date) {
    out << date.toString();
    return out;
}

std::istream& operator>>(std::istream& in,
                         Date& date) {
    int d, m, y;

    char separator1, separator2;

    in >> y >> separator1 >> m >> separator2 >> d;

    if (!in) {
        return in;
    }

    if (separator1 != '-' || separator2 != '-') {
        in.setstate(std::ios::failbit);
        return in;
    }

    try {
        date = Date(d, m, y);
    }
    catch (const InvalidDateException&) {
        in.setstate(std::ios::failbit);
    }

    return in;
}

}
}