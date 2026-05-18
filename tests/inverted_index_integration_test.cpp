#include "../src/index/inverted_index.hpp"
#include "../src/pipeline/html_parser.hpp"
#include "../src/pipeline/tokenizer.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::string read_file(const fs::path &path) {
	std::ifstream in(path, std::ios::binary);

    if (!in)
        return {};

    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

int main() {
    Tokenizer tokenizer;
    InvertedIndexBuilder builder;

    const fs::path input_dir = "data/small";

    uint32_t doc_id = 0;
    uint32_t processed = 0;

    for (const auto &entry : fs::directory_iterator(input_dir)) {
        if (!entry.is_regular_file())
            continue;

        if (entry.path().extension() != ".html")
            continue;

        std::string html = read_file(entry.path());

        if (html.empty())
            continue;

        auto parsed_result = parse_html(html);

        if (!parsed_result.has_value())
            continue;

        const ParsedDocument &parsed = parsed_result.value();
        auto tokens = tokenizer.tokenize(parsed.body_text);

        if (tokens.empty())
            continue;

        builder.add_document(doc_id++, tokens);
        processed++;

        if (processed >= 100)
            break;
    }

    builder.finalize();

    std::cout << "[info] indexed docs: " << builder.doc_count() << '\n';

    std::cout << "[info] avg doc length: " << builder.avg_doc_length() << '\n';

    const auto &index = builder.index();
    assert(!index.empty());

    auto it = index.find("search");
    assert(it != index.end());

    const auto &postings = it->second;

    assert(!postings.empty());

    for (size_t i = 1; i < postings.size(); i++)
        assert(postings[i - 1].doc_id < postings[i].doc_id);

    std::cout << "[info] term 'search' appears in " << postings.size() << " docs\n";
    std::cout << "[PASS] inverted_index_integration_test\n";

    return 0;
}
