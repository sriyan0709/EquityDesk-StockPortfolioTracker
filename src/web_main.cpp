#include "portfolio/web/WebServer.h"

#include <iostream>

int main() {
    try {
        portfolio::web::WebServer server;
        server.start();
    }
    catch (const std::exception& error) {
        std::cout << "Server error: " << error.what() << "\n";
        return 1;
    }

    return 0;
}