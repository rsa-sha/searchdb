#include <cassert>
#include <iostream>

#include "../src/crawler/url_frontier.hpp"

int main() {
    URLFrontier frontier;
    frontier.push("http://example.com");
    frontier.mark_done();

    // should STILL return pending URL
    auto a = frontier.pop();
    assert(a.has_value());
    // now frontier empty + done
    auto b = frontier.pop();

    assert(!b.has_value());
    std::cout << "[PASS] url_frontier_done\n";
    return 0;
}
