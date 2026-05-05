#include <iostream>
#include <atomic>
#include <thread>
#include "../src/common/thread_pool.hpp"

int main() {
    ThreadPool pool(8);

    std::atomic<int> counter = 0;

    std::vector<std::thread> producers;

    for (int t = 0; t < 4; t++) {
        producers.emplace_back([&] {
            for (int i = 0; i < 1000; i++) {
                pool.submit([&] {
                    counter.fetch_add(1, std::memory_order_relaxed);
                });
            }
        });
    }

    for (auto &t : producers)
        t.join();

    pool.shutdown();

    std::cout << "counter = " << counter.load() << "\n";

    return counter == 4000 ? 0 : 1;
}
