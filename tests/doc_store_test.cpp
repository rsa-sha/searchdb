#include <cassert>
#include <iostream>
#include <string>

#include "../src/pipeline/doc_store.hpp"

int main() {
    const std::string path = "test.docs";

    // ----------------------------
    // Write phase
    // ----------------------------

    DocStoreWriter writer;

    for (uint32_t i = 0; i < 100; ++i) {
        Document doc;

        doc.doc_id = i;

        doc.url =
            "https://example.com/page/" +
            std::to_string(i);

        doc.title =
            "Document Title " +
            std::to_string(i);

        doc.text =
            "This is the body text for document " +
            std::to_string(i);

        writer.add(doc);
    }

    auto write_result = writer.flush(path);

    assert(write_result.has_value());

    // ----------------------------
    // Read phase
    // ----------------------------

    DocStoreReader reader(path);

    assert(reader.doc_count() == 100);

    for (uint32_t i = 0; i < 100; ++i) {
        auto result = reader.get(i);

        assert(result.has_value());

        const Document& doc = result.value();

        assert(doc.doc_id == i);

        assert(
            doc.url ==
            "https://example.com/page/" +
            std::to_string(i));

        assert(
            doc.title ==
            "Document Title " +
            std::to_string(i));

        assert(
            doc.text ==
            "This is the body text for document " +
            std::to_string(i));
    }

    std::cout << "[PASS] doc_store round-trip test\n";

    return 0;
}
