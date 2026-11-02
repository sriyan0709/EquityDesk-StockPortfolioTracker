#ifndef PORTFOLIO_EXCEPTIONS_H
#define PORTFOLIO_EXCEPTIONS_H

#include <exception>
#include <string>

namespace portfolio {

class PortfolioException : public std::exception {
protected:
    std::string message;

public:
    explicit PortfolioException(const std::string& msg)
        : message(msg) {
    }

    virtual const char* what() const throw() {
        return message.c_str();
    }

    virtual ~PortfolioException() throw() {
    }
};

class InvalidDateException : public PortfolioException {
public:
    explicit InvalidDateException(const std::string& msg)
        : PortfolioException(msg) {
    }
};

class InsufficientSharesException : public PortfolioException {
public:
    explicit InsufficientSharesException(const std::string& msg)
        : PortfolioException(msg) {
    }
};

class InvalidSymbolException : public PortfolioException {
public:
    explicit InvalidSymbolException(const std::string& msg)
        : PortfolioException(msg) {
    }
};

class FileFormatException : public PortfolioException {
public:
    explicit FileFormatException(const std::string& msg)
        : PortfolioException(msg) {
    }
};

class PriceUnavailableException : public PortfolioException {
public:
    explicit PriceUnavailableException(const std::string& msg)
        : PortfolioException(msg) {
    }
};

class InvalidTransactionException : public PortfolioException {
public:
    explicit InvalidTransactionException(const std::string& msg)
        : PortfolioException(msg) {
    }
};

}

#endif