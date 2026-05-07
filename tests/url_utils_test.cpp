#include <cassert>
#include <iostream>

#include "../src/crawler/url_utils.hpp"

static void test_normalize_url() {
    {
        auto out = normalize_url("HTTP://Example.COM/Page/");
        assert(out == "http://example.com/Page");
    }

    {
        auto out = normalize_url("http://x.com/a#frag");
        assert(out == "http://x.com/a");
    }

    {
        auto out = normalize_url("http://x.com:80/a");
        assert(out == "http://x.com/a");
    }

    {
        auto out = normalize_url("https://x.com:443/a");
        assert(out == "https://x.com/a");
    }

    {
        auto out = normalize_url("https://Example.COM/");
        assert(out == "https://example.com/");
    }

    {
        auto out = normalize_url("mailto:test@example.com");
        assert(out.empty());
    }

    {
        auto out = normalize_url("javascript:alert(1)");
        assert(out.empty());
    }

    {
        auto out = normalize_url("ftp://example.com/file");
        assert(out.empty());
    }

    std::cout << "[PASS] normalize_url\n";
}

static void test_extract_domain() {
    {
        auto out = extract_domain("https://en.wikipedia.org/wiki/X");
        assert(out == "en.wikipedia.org");
    }

    {
        auto out = extract_domain("http://example.com");
        assert(out == "example.com");
    }

    {
        auto out = extract_domain("https://example.com:8080/a/b");
        assert(out == "example.com");
    }

    {
        auto out = extract_domain("https://127.0.0.1:3000/test");
        assert(out == "127.0.0.1");
    }

    {
        auto out = extract_domain("invalid-url");
        assert(out.empty());
    }

    std::cout << "[PASS] extract_domain\n";
}

static void test_resolve_url() {
    {
        auto out = resolve_url(
            "https://en.wikipedia.org/wiki/A",
            "/wiki/B"
        );

        assert(out == "https://en.wikipedia.org/wiki/B");
    }

    {
        auto out = resolve_url(
            "https://en.wikipedia.org/wiki/A",
            "B"
        );

        assert(out == "https://en.wikipedia.org/wiki/B");
    }

    {
        auto out = resolve_url(
            "https://example.com/a/b/c",
            "d"
        );

        assert(out == "https://example.com/a/b/d");
    }

    {
        auto out = resolve_url(
            "https://example.com/a/b",
            "https://google.com/x"
        );

        assert(out == "https://google.com/x");
    }

    {
        auto out = resolve_url(
            "https://example.com/a/b",
            "//cdn.example.com/file.js"
        );

        assert(out == "https://cdn.example.com/file.js");
    }

    {
        auto out = resolve_url(
            "https://example.com/a/b",
            "../c"
        );

        // depending on your implementation,
        // you may or may not support ".."
        // change expected accordingly
        assert(!out.empty());
    }

    std::cout << "[PASS] resolve_url\n";
}

int main() {
    test_normalize_url();
    test_extract_domain();
    test_resolve_url();

    std::cout << "\nAll url_utils tests passed.\n";

    return 0;
}
