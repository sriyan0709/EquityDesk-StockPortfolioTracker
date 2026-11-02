#include <iostream>

bool runPortfolioTests();
bool runStorageTests();
bool runPricingTests();

int main() {
    std::cout << "========================================\n";
    std::cout << " STOCK MARKET PORTFOLIO TRACKER TESTS\n";
    std::cout << "========================================\n\n";

    bool portfolioPassed = runPortfolioTests();

    std::cout << "\n\n========== STORAGE TESTS ==========\n\n";

    bool storagePassed = runStorageTests();

    std::cout << "\n\n========== PRICING TESTS ==========\n\n";

    bool pricingPassed = runPricingTests();

    if (portfolioPassed && storagePassed && pricingPassed) {
        std::cout << "\nALL TEST MODULES PASSED.\n";
        return 0;
    }

    std::cout << "\nSOME TEST MODULES FAILED.\n";
    return 1;
}