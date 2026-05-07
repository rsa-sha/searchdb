#include <cassert>
#include <iostream>

#include "../src/crawler/url_utils.hpp"

int main() {
	// root-relative test
	assert(resolve_url("https://en.wikipedia.org/wiki/A", "/wiki/B")
           == "https://en.wikipedia.org/wiki/B");
	// relative path
    assert(resolve_url("https://en.wikipedia.org/wiki/A", "B")
           == "https://en.wikipedia.org/wiki/B");

    assert(resolve_url("https://example.com/a/b/c", "d")
           == "https://example.com/a/b/d");

    assert(resolve_url("https://example.com/a/b", "https://google.com/x")
           == "https://google.com/x");

    assert(resolve_url("https://example.com/a/b", "//cdn.com/x.js")
           == "https://cdn.com/x.js");

    std::cout << "[PASS] url_resolve_test\n";
    return 0;
}
