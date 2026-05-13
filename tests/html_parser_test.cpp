#include "../src/pipeline/html_parser.hpp"

#include <cassert>
#include <iostream>

static void test_basic_html() {
    std::string html = R"(
        <html>
            <head>
                <title>Test Page</title>
            </head>
            <body>
                <p>Hello world</p>
                <a href="https://example.com">link</a>
            </body>
        </html>
    )";

    auto res = parse_html(html);

    assert(res.has_value());
    auto doc = res.value();

    assert(doc.title == "Test Page");
    assert(doc.body_text.find("Hello world") != std::string::npos);
    assert(doc.links.size() == 1);
    assert(doc.links[0] == "https://example.com");

    std::cout << "test_basic_html passed\n";
}

static void test_script_style_ignored() {
    std::string html = R"(
        <html>
            <body>
                <script>var x = 1;</script>
                <style>body { color: red; }</style>
                Visible text
            </body>
        </html>
    )";

    auto res = parse_html(html);

    assert(res.has_value());
    auto doc = res.value();

    assert(doc.body_text.find("var x") == std::string::npos);
    assert(doc.body_text.find("color") == std::string::npos);
    assert(doc.body_text.find("Visible text") != std::string::npos);

    std::cout << "test_script_style_ignored passed\n";
}

static void test_entities() {
    std::string html = R"(
        <html>
            <body>
                Tom &amp; Jerry &lt;Cartoon&gt;
            </body>
        </html>
    )";

    auto res = parse_html(html);

    assert(res.has_value());
    auto doc = res.value();

    assert(doc.body_text.find("Tom & Jerry") != std::string::npos);
    assert(doc.body_text.find("<Cartoon>") != std::string::npos);

    std::cout << "test_entities passed\n";
}

static void test_empty_html() {
    auto res = parse_html("");

    assert(res.has_value());
    auto doc = res.value();

    assert(doc.title.empty());
    assert(doc.body_text.empty());
    assert(doc.links.empty());

    std::cout << "test_empty_html passed\n";
}

int main() {
    test_basic_html();
    test_script_style_ignored();
    test_entities();
    test_empty_html();

    std::cout << "ALL TESTS PASSED\n";
}
