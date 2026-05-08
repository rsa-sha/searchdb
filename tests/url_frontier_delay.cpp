#include <cassert>
#include <chrono>
#include <iostream>

#include "../src/crawler/url_frontier.hpp"

int main() {
    URLFrontier frontier(std::chrono::milliseconds(50));
	for (int i = 0; i < 5; i++) {
        frontier.push("http://a.com/" + std::to_string(i));
        frontier.push("http://b.com/" + std::to_string(i));
    }
    frontier.mark_done();  // IMPORTANT for termination
    int count = 0;
    auto start = std::chrono::steady_clock::now();
    while (true) {
        auto url = frontier.pop();
        if (!url) break;
        count++;
    }
    auto end = std::chrono::steady_clock::now();
    assert(count == 10);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start
    ).count();
    assert(ms >= 50);
    std::cout << "[PASS] url_frontier_delay_stress\n";
}
