#include <cassert>
#include <iostream>
#include <string>

#include "../src/pipeline/doc_store.hpp"

int main() {
    const std::string path = "edge.docs";

    // ----------------------------
    // 1. Empty / minimal fields
    // ----------------------------
    {
        DocStoreWriter writer;

        Document doc;
        doc.doc_id = 0;
        doc.url = "";
        doc.title = "";
        doc.text = "";

        writer.add(doc);

        auto res = writer.flush(path);
        assert(res.has_value());

        DocStoreReader reader(path);

        assert(reader.doc_count() == 1);

        auto out = reader.get(0);
        assert(out.has_value());

        const Document& r = out.value();

        assert(r.doc_id == 0);
        assert(r.url.empty());
        assert(r.title.empty());
        assert(r.text.empty());
    }

    // ----------------------------
    // 2. Single large-ish document
    // ----------------------------
    {
        DocStoreWriter writer;

        Document doc;
        doc.doc_id = 42;
        doc.url = "https://example.com/" + std::string(1000, 'u');
        doc.title = std::string(2000, 't');
        doc.text = std::string(5000, 'x');

        writer.add(doc);

        auto res = writer.flush(path);
        assert(res.has_value());

        DocStoreReader reader(path);

        assert(reader.doc_count() == 1);

        auto out = reader.get(0);
        assert(out.has_value());

        const Document& r = out.value();

        assert(r.doc_id == 0); // stored index, not original doc_id
        assert(r.url == doc.url);
        assert(r.title == doc.title);
        assert(r.text == doc.text);
    }

    // ----------------------------
    // 3. Multiple docs + random access pattern
    // ----------------------------
    {
        DocStoreWriter writer;

        for (uint32_t i = 0; i < 10; ++i) {
            Document doc;
            doc.doc_id = i;
            doc.url = "u" + std::to_string(i);
            doc.title = "t" + std::to_string(i);
            doc.text = "x" + std::to_string(i);
            writer.add(doc);
        }

        auto res = writer.flush(path);
        assert(res.has_value());

        DocStoreReader reader(path);

        assert(reader.doc_count() == 10);

        // non-sequential access
        uint32_t order[] = {7, 0, 9, 3, 1, 6, 2, 8, 4, 5};

        for (uint32_t i : order) {
            auto out = reader.get(i);
            assert(out.has_value());

            const Document& r = out.value();

            assert(r.doc_id == i);
            assert(r.url == "u" + std::to_string(i));
            assert(r.title == "t" + std::to_string(i));
            assert(r.text == "x" + std::to_string(i));
        }
    }

    std::cout << "[PASS] doc_store edge-case tests\n";
    return 0;
}
