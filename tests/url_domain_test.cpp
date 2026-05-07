#include <cassert>
#include <iostream>

#include "../src/crawler/url_utils.hpp"

int main() {
    assert(extract_domain("https://en.wikipedia.org/wiki/X") == "en.wikipedia.org");
    assert(extract_domain("http://example.com") == "example.com");
    assert(extract_domain("https://example.com:8080/a") == "example.com");
    assert(extract_domain("https://127.0.0.1:3000/test") == "127.0.0.1");
    assert(extract_domain("invalid-url") == "");

    std::cout << "[PASS] url_domain_test\n";
    return 0;
}
