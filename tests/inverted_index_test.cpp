#include "../src/index/inverted_index.hpp"

#include <cassert>
#include <iostream>

int main() {
    InvertedIndexBuilder builder;

    builder.add_document(0, {"search", "engine", "search"});
    builder.add_document(1, {"search", "database"});
    builder.finalize();

    const auto &index = builder.index();
    // Verify "search"
    {
        const auto &postings = index.at("search");

        assert(postings.size() == 2);

        assert(postings[0].doc_id == 0);
        assert(postings[0].term_freq == 2);

        assert(postings[1].doc_id == 1);
        assert(postings[1].term_freq == 1);
    }
    // Verify "engine"
    {
        const auto &postings = index.at("engine");
        assert(postings.size() == 1);

        assert(postings[0].doc_id == 0);
        assert(postings[0].term_freq == 1);
    }
    // Verify counts
    assert(builder.doc_count() == 2);
    // avg = (3 + 2) / 2 = 2.5
    assert(builder.avg_doc_length() == 2.5);
    std::cout << "[PASS] inverted_index_test\n";
    return 0;
}
