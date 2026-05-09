#include "../src/crawler/robots.hpp"

#include <cassert>
#include <iostream>

int main() {
    RobotsChecker checker;

    auto delay =
        checker.crawl_delay("en.wikipedia.org");

    // wikipedia may not define crawl-delay,
    // so default should be >= 1000ms

    assert(delay.count() >= 1000);

    std::cout << "crawl delay: "
              << delay.count()
              << "ms\n";

    std::cout << "robots_delay_test passed\n";
}
