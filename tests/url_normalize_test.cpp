#include <cassert>
#include <iostream>

#include "../src/crawler/url_utils.hpp"

int main() {
    // case normalization (scheme + host + path)
    assert(normalize_url("HTTP://Example.COM/Page/") == "http://example.com/Page");

    // fragment removal
    assert(normalize_url("http://x.com/a#frag") == "http://x.com/a");

    // default port removal
    assert(normalize_url("http://x.com:80/a") == "http://x.com/a");

    // https default port removal
    assert(normalize_url("https://x.com:443/a") == "https://x.com/a");

    std::cout << "[PASS] url_normalize_test\n";
    return 0;
}
