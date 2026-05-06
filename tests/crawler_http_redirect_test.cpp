#include <cassert>
#include <iostream>

#include "../src/crawler/http_client.hpp"

int main() {
    std::cout << "Running redirect test...\n";

    // GitHub HTTP -> HTTPS redirect
    auto res = fetch("http://github.com");

    assert(res.has_value());

    const auto& resp = res.value();

    // After redirect, we should land on HTTPS URL
    std::cout << "Final URL: " << resp.final_url << "\n";
    std::cout << "Status: " << resp.status_code << "\n";

    assert(resp.status_code == 200);
    assert(resp.final_url.find("https://github.com") != std::string::npos);

    std::cout << "Redirect test passed\n";
    return 0;
}
