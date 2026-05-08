#include <atomic>
#include <cassert>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>

#include "../src/crawler/url_frontier.hpp"

int main() {
    URLFrontier frontier;
    constexpr int N = 200;
    for (int i = 0; i < N; ++i) {
        frontier.push("http://example.com/page/" + std::to_string(i));
    }
    frontier.mark_done();

    std::unordered_set<std::string> popped;
    std::mutex mu;
    std::atomic<int> pop_count = 0;

    auto worker = [&]() {
        while (true) {
            auto url = frontier.pop();
            if (!url) break;
            {
                std::lock_guard lock(mu);
                assert(!popped.contains(*url));
                popped.insert(*url);
            }
            pop_count++;
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i)
        threads.emplace_back(worker);
    for (auto &t : threads)
        t.join();

    assert(pop_count == N);
    assert(popped.size() == N);

    std::cout << "[PASS] url_frontier_threaded\n";
}
