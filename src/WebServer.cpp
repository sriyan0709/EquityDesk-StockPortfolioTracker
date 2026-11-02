#include "portfolio/web/WebServer.h"

#include "portfolio/portfolio/Portfolio.h"
#include "portfolio/storage/TransactionStore.h"
#include "portfolio/storage/TransactionRecord.h"
#include "portfolio/pricing/CsvPriceProvider.h"
#include "portfolio/pricing/PriceQuote.h"
#include "portfolio/core/Date.h"
#include "portfolio/core/Money.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cctype>

#pragma comment(lib, "ws2_32.lib")

namespace portfolio {
namespace web {

using namespace portfolio::core;
using namespace portfolio::portfolio_data;
using namespace portfolio::storage;
using namespace portfolio::pricing;
using namespace portfolio::transactions;

static const char* TRANSACTION_FILE = "data/transactions.dat";
static const char* PRICE_FILE = "data/prices.csv";

std::string readFile(const std::string& path) {
    std::ifstream file(path.c_str(), std::ios::binary);

    if (!file.is_open()) {
        return "";
    }

    std::ostringstream contents;
    contents << file.rdbuf();

    return contents.str();
}

std::string jsonEscape(const std::string& value) {
    std::string result;

    for (std::size_t i = 0; i < value.size(); ++i) {
        char c = value[i];

        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else if (c == '\t') result += "\\t";
        else if (static_cast<unsigned char>(c) >= 32) result += c;
    }

    return result;
}

std::string moneyJson(const Money& value) {
    std::ostringstream out;

    out << value.getPaise() / 100.0;

    return out.str();
}

std::string transactionTypeName(TxnType type) {
    switch (type) {
        case BUY: return "BUY";
        case SELL: return "SELL";
        case DIVIDEND: return "DIVIDEND";
        case SPLIT: return "SPLIT";
        case BONUS: return "BONUS";
        default: return "UNKNOWN";
    }
}

bool loadPortfolio(
    TransactionStore& store,
    Portfolio& portfolio
) {
    if (!store.loadFromFile(TRANSACTION_FILE)) {
        std::ifstream check(TRANSACTION_FILE);

        if (check.good()) {
            return false;
        }

        return true;
    }

    store.replayInto(portfolio);
    return true;
}

std::string buildState() {
    TransactionStore store;
    Portfolio portfolio;

    if (!loadPortfolio(store, portfolio)) {
        return "{\"error\":\"Could not load transaction storage.\"}";
    }

    CsvPriceProvider prices(PRICE_FILE);

    for (int i = 0; i < portfolio.getHoldingCount(); ++i) {
        Holding& holding = portfolio.getHoldingAt(i);

        PriceQuote quote;

        if (prices.getPrice(holding.getSymbol(), quote)) {
            holding.setLastPrice(quote.getPrice());
        }
    }

    std::ostringstream out;

    out << "{";

    out << "\"summary\":{";
    out << "\"invested\":" << moneyJson(portfolio.getTotalCost()) << ",";
    out << "\"marketValue\":" << moneyJson(portfolio.getTotalMarketValue()) << ",";
    out << "\"realisedPL\":" << moneyJson(portfolio.getTotalRealisedProfitLoss()) << ",";
    out << "\"unrealisedPL\":" << moneyJson(portfolio.getTotalUnrealisedProfitLoss()) << ",";
    out << "\"dividends\":" << moneyJson(portfolio.getTotalDividendIncome()) << ",";
    out << "\"totalReturn\":" << moneyJson(portfolio.getTotalReturn()) << ",";
    out << "\"holdingCount\":" << portfolio.getHoldingCount() << ",";
    out << "\"shareCount\":" << portfolio.getTotalShareCount();
    out << "},";

    out << "\"holdings\":[";

    for (int i = 0; i < portfolio.getHoldingCount(); ++i) {
        const Holding& h = portfolio.getHoldingAt(i);

        if (i > 0) out << ",";

        out << "{";
        out << "\"symbol\":\"" << jsonEscape(h.getSymbol()) << "\",";
        out << "\"quantity\":" << h.getQuantity() << ",";
        out << "\"averageCost\":" << moneyJson(h.getAverageCostPerShare()) << ",";
        out << "\"lastPrice\":" << moneyJson(h.getLastPrice()) << ",";
        out << "\"totalCost\":" << moneyJson(h.getTotalCost()) << ",";
        out << "\"marketValue\":" << moneyJson(h.getMarketValue()) << ",";
        out << "\"unrealisedPL\":" << moneyJson(h.getUnrealisedProfitLoss()) << ",";
        out << "\"realisedPL\":" << moneyJson(h.getRealisedProfitLoss());
        out << "}";
    }

    out << "],";

    out << "\"transactions\":[";

    for (int i = 0; i < store.size(); ++i) {
        const TransactionRecord& r = store.get(i);

        if (i > 0) out << ",";

        out << "{";
        out << "\"id\":" << r.transactionId << ",";
        out << "\"type\":\"" << transactionTypeName(r.type) << "\",";
        out << "\"symbol\":\"" << jsonEscape(r.symbol) << "\",";
        out << "\"date\":\"" << r.date.toString() << "\",";
        out << "\"quantity\":" << r.quantity << ",";
        out << "\"price\":" << (r.pricePaise / 100.0) << ",";
        out << "\"fees\":" << (r.feesPaise / 100.0) << ",";
        out << "\"numerator\":" << r.numerator << ",";
        out << "\"denominator\":" << r.denominator;
        out << "}";
    }

    out << "]";

    out << "}";

    return out.str();
}

std::string getContentType(const std::string& path) {
    if (path.find(".css") != std::string::npos) {
        return "text/css";
    }

    if (path.find(".js") != std::string::npos) {
        return "application/javascript";
    }

    if (path.find(".html") != std::string::npos) {
        return "text/html";
    }

    return "text/plain";
}

void sendResponse(
    SOCKET client,
    int status,
    const std::string& contentType,
    const std::string& body
) {
    std::ostringstream response;

    response << "HTTP/1.1 " << status << " ";

    if (status == 200) response << "OK";
    else if (status == 400) response << "Bad Request";
    else if (status == 404) response << "Not Found";
    else response << "Internal Server Error";

    response << "\r\n";
    response << "Content-Type: " << contentType << "; charset=utf-8\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Access-Control-Allow-Origin: http://127.0.0.1:8080\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    std::string result = response.str();

    send(
        client,
        result.c_str(),
        static_cast<int>(result.size()),
        0
    );
}

std::string getRequestBody(const std::string& request) {
    std::string::size_type position = request.find("\r\n\r\n");

    if (position == std::string::npos) {
        return "";
    }

    return request.substr(position + 4);
}

std::string getJsonString(
    const std::string& body,
    const std::string& key
) {
    std::string search = "\"" + key + "\"";

    std::string::size_type position = body.find(search);

    if (position == std::string::npos) {
        return "";
    }

    position = body.find(':', position);

    if (position == std::string::npos) {
        return "";
    }

    position = body.find('"', position);

    if (position == std::string::npos) {
        return "";
    }

    std::string::size_type end = body.find('"', position + 1);

    if (end == std::string::npos) {
        return "";
    }

    return body.substr(position + 1, end - position - 1);
}

long long getJsonNumber(
    const std::string& body,
    const std::string& key,
    long long defaultValue = 0
) {
    std::string search = "\"" + key + "\"";

    std::string::size_type position = body.find(search);

    if (position == std::string::npos) {
        return defaultValue;
    }

    position = body.find(':', position);

    if (position == std::string::npos) {
        return defaultValue;
    }

    ++position;

    while (position < body.size() &&
           std::isspace(static_cast<unsigned char>(body[position]))) {
        ++position;
    }

    char* end = 0;

    double value = std::strtod(body.c_str() + position, &end);

    if (end == body.c_str() + position) {
        return defaultValue;
    }

    return static_cast<long long>(value * 100.0);
}

bool getJsonInteger(
    const std::string& body,
    const std::string& key,
    int& result
) {
    std::string search = "\"" + key + "\"";

    std::string::size_type position = body.find(search);

    if (position == std::string::npos) {
        return false;
    }

    position = body.find(':', position);

    if (position == std::string::npos) {
        return false;
    }

    ++position;

    while (position < body.size() &&
           std::isspace(static_cast<unsigned char>(body[position]))) {
        ++position;
    }

    char* end = 0;

    long value = std::strtol(body.c_str() + position, &end, 10);

    if (end == body.c_str() + position) {
        return false;
    }

    if (value < 0 || value > 2147483647L) {
        return false;
    }

    result = static_cast<int>(value);
    return true;
}

std::string saveTransaction(const std::string& body) {
    try {
        TransactionStore store;
        Portfolio portfolio;

        if (!loadPortfolio(store, portfolio)) {
            return "{\"error\":\"Could not load existing transactions.\"}";
        }

        TransactionRecord record;

        std::string type = getJsonString(body, "type");
        record.symbol = getJsonString(body, "symbol");

        std::string dateText = getJsonString(body, "date");

        std::istringstream dateInput(dateText);

        if (!(dateInput >> record.date) || !record.date.isValid()) {
            return "{\"error\":\"Invalid date. Use YYYY-MM-DD.\"}";
        }

        if (record.symbol.empty()) {
            return "{\"error\":\"Stock symbol is required.\"}";
        }

        for (std::size_t i = 0; i < record.symbol.size(); ++i) {
            record.symbol[i] = static_cast<char>(
                std::toupper(
                    static_cast<unsigned char>(record.symbol[i])
                )
            );
        }

        int quantity = 0;

        if (!getJsonInteger(body, "quantity", quantity)) {
            quantity = 0;
        }

        long long pricePaise = getJsonNumber(body, "price");
        long long feesPaise = getJsonNumber(body, "fees");

        record.quantity = quantity;
        record.pricePaise = pricePaise;
        record.feesPaise = feesPaise;

        long highestId = 0;

        for (int i = 0; i < store.size(); ++i) {
            if (store.get(i).transactionId > highestId) {
                highestId = store.get(i).transactionId;
            }
        }

        record.transactionId = highestId + 1;

        if (type == "BUY") {
            record.type = BUY;
        } else if (type == "SELL") {
            record.type = SELL;
        } else if (type == "DIVIDEND") {
            record.type = DIVIDEND;
        } else if (type == "SPLIT") {
            record.type = SPLIT;
        } else if (type == "BONUS") {
            record.type = BONUS;
        } else {
            return "{\"error\":\"Unsupported transaction type.\"}";
        }

        if (record.type == SPLIT || record.type == BONUS) {
            if (!getJsonInteger(body, "numerator", record.numerator) ||
                !getJsonInteger(body, "denominator", record.denominator)) {
                return "{\"error\":\"Split or bonus ratio is required.\"}";
            }
        }

        store.addRecord(record);

        // Validate the entire transaction history before saving.
        Portfolio validationPortfolio;
        store.replayInto(validationPortfolio);

        if (!store.saveToFile(TRANSACTION_FILE)) {
            return "{\"error\":\"Could not save transaction file.\"}";
        }

        return "{\"success\":true,\"message\":\"Transaction saved.\"}";
    }
    catch (const std::exception& error) {
        return std::string("{\"error\":\"") +
            jsonEscape(error.what()) + "\"}";
    }
    catch (...) {
        return "{\"error\":\"An unknown error occurred.\"}";
    }
}

void handleClient(SOCKET client) {
    char buffer[16384];

    int received = recv(client, buffer, sizeof(buffer) - 1, 0);

    if (received <= 0) {
        closesocket(client);
        return;
    }

    buffer[received] = '\0';

    std::string request(buffer);

    std::istringstream requestLine(request);

    std::string method;
    std::string path;
    std::string protocol;

    requestLine >> method >> path >> protocol;

    if (method == "OPTIONS") {
        sendResponse(client, 200, "text/plain", "");
    }
    else if (method == "GET" && path == "/api/state") {
        sendResponse(client, 200, "application/json", buildState());
    }
    else if (method == "POST" && path == "/api/transactions") {
        std::string body = getRequestBody(request);

        std::string result = saveTransaction(body);

        if (result.find("\"success\":true") != std::string::npos) {
            sendResponse(client, 200, "application/json", result);
        } else {
            sendResponse(client, 400, "application/json", result);
        }
    }
    else if (method == "GET") {
        if (path == "/") {
            path = "/index.html";
        }

        if (path.find("..") != std::string::npos ||
            path.find('\\') != std::string::npos) {
            sendResponse(client, 400, "text/plain", "Invalid path.");
        }
        else {
            std::string filePath = "EquityDesk" + path;
            std::string content = readFile(filePath);

            if (content.empty()) {
                sendResponse(client, 404, "text/plain", "File not found.");
            } else {
                sendResponse(
                    client,
                    200,
                    getContentType(filePath),
                    content
                );
            }
        }
    }
    else {
        sendResponse(client, 404, "text/plain", "Endpoint not found.");
    }

    closesocket(client);
}

void WebServer::start() {
    WSADATA data;

    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        std::cout << "Winsock initialization failed.\n";
        return;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Could not create server socket.\n";
        WSACleanup();
        return;
    }

    sockaddr_in serverAddress;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(
            serverSocket,
            reinterpret_cast<sockaddr*>(&serverAddress),
            sizeof(serverAddress)
        ) == SOCKET_ERROR) {

        std::cout << "Could not bind to port 8080.\n";
        std::cout << "Check whether another server is using this port.\n";

        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "Could not start listening.\n";

        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    std::cout << "\n====================================\n";
    std::cout << "       EQUITYDESK WEB SERVER\n";
    std::cout << "====================================\n";
    std::cout << "Open: http://127.0.0.1:8080\n";
    std::cout << "Keep this terminal open.\n";
    std::cout << "Press Ctrl+C to stop.\n\n";

    while (true) {
        SOCKET client = accept(serverSocket, 0, 0);

        if (client == INVALID_SOCKET) {
            continue;
        }

        handleClient(client);
    }

    closesocket(serverSocket);
    WSACleanup();
}

}
}