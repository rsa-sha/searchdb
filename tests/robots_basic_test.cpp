#include "../src/crawler/robots.hpp"

#include <cassert>
#include <iostream>

int main() {
    RobotsChecker checker;

    // wikipedia disallows /wiki/Special:
    bool allowed =
        checker.is_allowed(
            "https://en.wikipedia.org/wiki/C%2B%2B");

    bool blocked =
        checker.is_allowed(
            "https://en.wikipedia.org/wiki/Special:Random");

    assert(allowed == true);
    assert(blocked == false);

    std::cout << "robots_basic_test passed\n";
}
