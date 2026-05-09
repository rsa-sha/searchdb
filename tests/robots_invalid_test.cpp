#include "../src/crawler/robots.hpp"

#include <cassert>
#include <iostream>

int main() {
    RobotsChecker checker;

    // invalid domain should fail open
    bool allowed =
        checker.is_allowed(
            "https://this-domain-should-not-exist-12345.com/page");

    assert(allowed == true);

    std::cout << "robots_invalid_test passed\n";
}
