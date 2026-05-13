#include "../src/pipeline/html_parser.hpp"
#include "../src/pipeline/tokenizer.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

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

static void test_real_html_tokenization() {
    namespace fs = std::filesystem;

    const std::string dir = "data/raw";

    if (!fs::exists(dir)) {
        std::cout << "\n[SKIP] data/raw not found.\n"
                  << "Run crawler first:\n"
                  << "./searchdb crawl --seeds=data/seeds.txt --max-pages=100 --threads=4\n";
        return;
    }

    Tokenizer tokenizer;

    int found_files = 0;

    for (int i = 0; i < 5; ++i) {
        std::string path = dir + "/" + std::to_string(i) + ".html";

        if (!fs::exists(path))
            continue;

        found_files++;

        std::string html = read_file(path);

        auto res = parse_html(html);
        assert(res.has_value());

        auto doc = res.value();

        auto tokens = tokenizer.tokenize(doc.body_text);

        std::cout << "\n=================================\n";
        std::cout << "FILE: " << path << "\n";
        std::cout << "TITLE: " << doc.title << "\n";

        std::cout << "TOKENS (first 100):\n";

        for (size_t j = 0; j < std::min<size_t>(100, tokens.size()); ++j) {
            std::cout << tokens[j] << " ";
        }

        std::cout << "\nTOKEN COUNT: " << tokens.size() << "\n";

        // sanity checks
        assert(!doc.body_text.empty());

        // ensure tokenizer is actually doing something useful
        if (!tokens.empty()) {
            for (const auto &t : tokens) {
                assert(t.size() >= 2); // split rule consistency
            }
        }
    }

    if (found_files == 0) {
        std::cout << "\n[SKIP] No html files found in data/raw\n"
                  << "Run crawler first:\n"
                  << "./searchdb crawl --seeds=data/seeds.txt --max-pages=100 --threads=4\n";
        return;
    }

    std::cout << "\ntest_real_html_tokenization passed\n";
}

int main() {
    test_real_html_tokenization();
    std::cout << "ALL TOKENIZER TESTS PASSED\n";
}
