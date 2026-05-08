#include <cassert>
#include <iostream>
#include <unordered_set>

#include "../src/crawler/url_frontier.hpp"

int main() {
    URLFrontier frontier;

    frontier.push("http://example.com/a");
    frontier.push("http://example.com/a"); // duplicate
    frontier.push("https://google.com/b");

    // dedup check: only 2 unique URLs
    assert(frontier.seen() == 2);
    assert(frontier.pending() == 2);

	frontier.mark_done();

    std::unordered_set<std::string> popped;

    while (auto url = frontier.pop()) {
        assert(!popped.contains(*url)); // no duplicates from pop()
        popped.insert(*url);
    }

    assert(popped.size() == 2);
    assert(frontier.pending() == 0);

    std::cout << "[PASS] url_frontier_basic\n";
}
