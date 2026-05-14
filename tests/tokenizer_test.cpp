#include "../src/pipeline/tokenizer.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

static void test_basic_tokens() {
    Tokenizer tokenizer;

    std::string text = "The Quick Brown Fox";
    auto tokens = tokenizer.tokenize(text);

    assert(tokens.size() == 3);
    assert(tokens[0] == "quick");
    assert(tokens[1] == "brown");
    assert(tokens[2] == "fox");

    std::cout << "test_basic_tokens passed\n";
}

static void test_punctuation_and_numbers() {
    Tokenizer tokenizer;

    std::string text = "C++ programming in 2024!";
    auto tokens = tokenizer.tokenize(text);

    assert(tokens.size() == 2);
	// This is stemmed now
    assert(tokens[0] == "program");
    assert(tokens[1] == "2024");

    std::cout << "test_punctuation_and_numbers passed\n";
}

static void test_stop_words_removed() {
    Tokenizer tokenizer;

    std::string text = "This is a simple test of stop words removal";
    auto tokens = tokenizer.tokenize(text);

    for (const auto& t : tokens) {
        assert(t != "this" && t != "is" && t != "a" && t != "of");
    }

    std::cout << "test_stop_words_removed passed\n";
}

static void test_tokenize_query_consistency() {
    Tokenizer tokenizer;

    std::string doc_text = "Natural language processing in 2024";
    std::string query_text = "Processing language in 2024";

    auto doc_tokens = tokenizer.tokenize(doc_text);
    auto query_tokens = tokenizer.tokenize_query(query_text);

    // Both pipelines lowercase and remove stop words, also words are being passed through a stemmer
    assert(std::find(doc_tokens.begin(), doc_tokens.end(), "process") != doc_tokens.end());
    assert(std::find(query_tokens.begin(), query_tokens.end(), "process") != query_tokens.end());

    std::cout << "test_tokenize_query_consistency passed\n";
}

int main() {
    test_basic_tokens();
    test_punctuation_and_numbers();
    test_stop_words_removed();
    test_tokenize_query_consistency();

    std::cout << "ALL TOKENIZER TESTS PASSED\n";
}
