#include "../src/index/inverted_index.hpp"
#include "../src/index/doc_lengths.hpp"
#include "../src/search/bm25_scorer.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

static DocLengthReader make_lengths(const std::vector<uint32_t>& v) {
    static int id = 0;
    std::string path = "test_lengths_" + std::to_string(id++) + ".bin";

    DocLengthWriter writer;
    writer.write(path, v);

    return DocLengthReader(path);
}

static void test_idf_behavior() {
    auto reader = make_lengths({100});

    BM25Scorer scorer(
        5000,
        500.0,
        &reader
    );

    double rare = scorer.idf(5);
    double common = scorer.idf(4900);

    assert(rare > common);

    std::cout << "[PASS] idf behavior\n";
}

static void test_tf_saturation() {
    auto reader = make_lengths({500});

    BM25Scorer scorer(
        5000,
        500.0,
        &reader
    );

    double tf1 = scorer.score_term(1, 500);
    double tf10 = scorer.score_term(10, 500);
    double tf100 = scorer.score_term(100, 500);

    assert(tf10 > tf1);
    assert(tf100 > tf10);
    assert(tf100 < tf10 * 3.0);

    std::cout << "[PASS] tf saturation\n";
}

static void test_length_normalization() {
    auto reader = make_lengths({100, 1000});

    BM25Scorer scorer(
        5000,
        500.0,
        &reader
    );

    double short_doc =
        scorer.score_term(5, 100);

    double long_doc =
        scorer.score_term(5, 1000);

    assert(short_doc > long_doc);

    std::cout << "[PASS] length normalization\n";
}

static void test_single_term_query() {
    InvertedIndexBuilder index;

    index.add_document(0, {"machine", "learning"});
    index.add_document(1, {"machine", "machine", "machine"});
    index.add_document(2, {"gardening", "flowers"});

    index.finalize();

    auto reader = make_lengths(index.doc_lengths());

    BM25Scorer scorer(
        index.doc_count(),
        index.avg_doc_length(),
        &reader
    );

    auto results = scorer.query({"machine"}, index, 10);

    assert(results.size() >= 2);
    assert(results[0].doc_id == 1);
    assert(results[1].doc_id == 0);

    std::cout << "[PASS] single-term query ranking\n";
}

static void test_multi_term_query() {
    InvertedIndexBuilder index;

    index.add_document(0, {"machine", "learning"});
    index.add_document(1, {"machine"});
    index.add_document(2, {"learning"});

    index.finalize();

    auto reader = make_lengths(index.doc_lengths());

    BM25Scorer scorer(
        index.doc_count(),
        index.avg_doc_length(),
        &reader
    );

    auto results =
        scorer.query({"machine", "learning"}, index, 10);

    assert(!results.empty());
    assert(results[0].doc_id == 0);

    std::cout << "[PASS] multi-term accumulation\n";
}

static void test_missing_term() {
    InvertedIndexBuilder index;

    index.add_document(0, {"hello"});
    index.finalize();

    auto reader = make_lengths(index.doc_lengths());

    BM25Scorer scorer(
        index.doc_count(),
        index.avg_doc_length(),
        &reader
    );

    auto results = scorer.query({"nonexistent"}, index);

    assert(results.empty());

    std::cout << "[PASS] missing term handling\n";
}

int main() {
    test_idf_behavior();
    test_tf_saturation();
    test_length_normalization();
    test_single_term_query();
    test_multi_term_query();
    test_missing_term();

    std::cout << "[PASS] bm25_scorer_integration_test\n";
    return 0;
}
