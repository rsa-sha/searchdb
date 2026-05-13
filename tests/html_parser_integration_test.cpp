#include "../src/pipeline/html_parser.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

static std::string read_file(const std::string &path) {
    std::ifstream in(path, std::ios::binary);

    if (!in.is_open()) {
        throw std::runtime_error(
            "failed to open file: " + path
        );
    }

    return std::string(
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>()
    );
}


static void test_real_crawled_html() {
    namespace fs = std::filesystem;

    const std::string dir = "data/raw";

    if (!fs::exists(dir)) {
        std::cout << "\n[SKIP] data/raw not found.\n" << "Run crawler first:\n"
            << "./searchdb crawl " << "--seeds=data/seeds.txt " << "--max-pages=100 " << "--threads=4\n";

        return;
    }

    int found_files = 0;

    for (int i = 0; i < 5; ++i) {
        std::string path =
            dir + "/" + std::to_string(i) + ".html";

        if (!fs::exists(path))
            continue;

        found_files++;

        std::string html = read_file(path);

        auto res = parse_html(html);

        assert(res.has_value());

        auto doc = res.value();

        std::cout << "\n=================================\n";
        std::cout << "FILE: " << path << "\n";
        std::cout << "TITLE: " << doc.title << "\n";
        std::cout << "BODY PREVIEW:\n";

        std::string preview = doc.body_text.substr(0, std::min<size_t>(200, doc.body_text.size()));
        std::cout << preview << "\n";

        std::cout << "LINK COUNT: " << doc.links.size() << "\n";

        assert(!doc.body_text.empty());
    }

    if (found_files == 0) {
        std::cout << "\n[SKIP] No html files found in data/raw\n" << "Run crawler first:\n"
            << "./searchdb crawl " << "--seeds=data/seeds.txt " << "--max-pages=100 " << "--threads=4\n";
        return;
    }

    std::cout << "test_real_crawled_html passed\n";
}

int main() {
    test_real_crawled_html();
    std::cout << "ALL TESTS PASSED\n";
}
