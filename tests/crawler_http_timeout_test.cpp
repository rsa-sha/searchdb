#include <cassert>
#include <iostream>

#include "../src/crawler/http_client.hpp"

int main() {
    std::cout << "Running timeout test...\n";

    // This IP is non-routable → will hang until timeout
    auto res = fetch("http://10.255.255.1");

    // Should fail
    assert(!res.has_value());

    std::cout << "Timeout test triggered error correctly\n";
    return 0;
}
