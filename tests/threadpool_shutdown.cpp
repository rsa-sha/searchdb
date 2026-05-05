#include <iostream>
#include <atomic>
#include <thread>
#include "../src/common/thread_pool.hpp"

int main() {
    ThreadPool pool(4);

    std::atomic<int> counter = 0;

    for (int i = 0; i < 200; i++) {
        pool.submit([&] {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            counter++;
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    pool.shutdown();

    std::cout << "counter = " << counter.load() << "\n";

    return 0;
}
