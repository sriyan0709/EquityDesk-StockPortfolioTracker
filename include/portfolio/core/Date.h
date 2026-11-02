#ifndef PORTFOLIO_DATE_H
#define PORTFOLIO_DATE_H

#include <string>
#include <iostream>

namespace portfolio {
namespace core {

class Date {
private:
    int day;
    int month;
    int year;

    static bool isLeapYear(int year);
    static int daysInMonth(int month, int year);

public:
    Date();
    Date(int day, int month, int year);

    int getDay() const;
    int getMonth() const;
    int getYear() const;

    bool isValid() const;
    long long serialDay() const;

    std::string toString() const;

    bool operator<(const Date& other) const;
    bool operator>(const Date& other) const;
    bool operator<=(const Date& other) const;
    bool operator>=(const Date& other) const;
    bool operator==(const Date& other) const;
    bool operator!=(const Date& other) const;

    Date& operator++();

    friend std::ostream& operator<<(std::ostream& out,
                                    const Date& date);

    friend std::istream& operator>>(std::istream& in,
                                    Date& date);
};

}
}

#endif