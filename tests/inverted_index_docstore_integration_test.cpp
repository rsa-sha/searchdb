#include "../src/index/inverted_index.hpp"
#include "../src/pipeline/doc_store.hpp"
#include "../src/pipeline/tokenizer.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int main() {
    DocStoreReader reader("data/processed/docs.bin");

    Tokenizer tokenizer;
    InvertedIndexBuilder builder;

    uint32_t total_docs = reader.doc_count();

    // Sanity guard: ensure we are testing real pipeline scale
    assert(total_docs > 0);

    for (uint32_t doc_id = 0; doc_id < total_docs; ++doc_id) {
        auto result = reader.get(doc_id);
        if (!result.has_value())
            continue;

        const Document &doc = result.value();

        std::string combined = doc.title + " " + doc.text;
        auto tokens = tokenizer.tokenize(combined);

        if (tokens.empty())
            continue;

        builder.add_document(doc_id, tokens);
    }

    builder.finalize();

    // -----------------------------
    // Core structural validation
    // -----------------------------
    assert(builder.doc_count() == total_docs);

    const auto &index = builder.index();
    assert(!index.empty());

    auto validate_term = [&](const std::string &term) {
        auto it = index.find(term);
        assert(it != index.end());

        const auto &postings = it->second;
        assert(!postings.empty());

        for (size_t i = 1; i < postings.size(); ++i) {
            assert(postings[i - 1].doc_id < postings[i].doc_id);
        }

        std::cout << "[check] term '" << term
                  << "' appears in " << postings.size()
                  << " documents\n";
    };

    // -----------------------------
    // Deterministic “golden terms”
    // -----------------------------
    validate_term("the");
    validate_term("http");
    validate_term("wikipedia");

    // Optional semantic sanity check (lower frequency, more meaningful)
    validate_term("algorithm");

    // -----------------------------
    // Stats sanity (not exact, just bounds)
    // -----------------------------
    std::cout << "[info] docs indexed: " << builder.doc_count() << "\n";
    std::cout << "[info] avg doc length: " << builder.avg_doc_length() << "\n";

    assert(builder.avg_doc_length() > 0.0);

    std::cout << "[PASS] inverted_index_docstore_integration_test\n";
    return 0;
}
