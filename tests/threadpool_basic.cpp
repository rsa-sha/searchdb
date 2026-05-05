#include <iostream>
#include <atomic>
#include "../src/common/thread_pool.hpp"

int main() {
    ThreadPool pool(4);

    std::atomic<int> counter = 0;

    for (int i = 0; i < 100; i++) {
        pool.submit([&] {
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }

    pool.shutdown();

    std::cout << "counter = " << counter.load() << "\n";

    return counter == 100 ? 0 : 1;
}
