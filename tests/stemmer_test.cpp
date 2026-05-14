#include "../src/pipeline/tokenizer.hpp"

#include <cassert>
#include <iostream>

static void test_individual_stems() {
    Tokenizer tokenizer;

    assert(tokenizer.stem_public("running") == "run");
    assert(tokenizer.stem_public("studies") == "study");
    assert(tokenizer.stem_public("carefully") == "care");
    assert(tokenizer.stem_public("management") == "manage");
    assert(tokenizer.stem_public("darkness") == "dark");
    assert(tokenizer.stem_public("cats") == "cat");
    assert(tokenizer.stem_public("glass") == "glass");
    assert(tokenizer.stem_public("king") == "king");
    assert(tokenizer.stem_public("bed") == "bed");
    assert(tokenizer.stem_public("updated") == "updat");

    std::cout << "test_individual_stems passed\n";
}

static void test_priority_rules() {
    Tokenizer tokenizer;

    // ensures -ies beats -s
    assert(tokenizer.stem_public("studies") == "study");

    // ensures -es works
    assert(tokenizer.stem_public("cares") == "car");

    // ensure no partial truncation
    assert(tokenizer.stem_public("running") == "run");

    std::cout << "test_priority_rules passed\n";
}

static void test_no_overstemming() {
    Tokenizer tokenizer;

    assert(tokenizer.stem_public("is") == "is");
    assert(tokenizer.stem_public("be") == "be");

    assert(tokenizer.stem_public("king") == "king");
    assert(tokenizer.stem_public("bed") == "bed");

    std::cout << "test_no_overstemming passed\n";
}

static void test_tokenizer_integration() {
    Tokenizer tokenizer;

    std::string text = "Running studies carefully and updated systems";

    auto tokens = tokenizer.tokenize(text);

    bool has_run = false;
    bool has_study = false;
    bool has_care = false;
    bool has_updat = false;

    for (auto &t : tokens) {
        if (t == "run") has_run = true;
        if (t == "study") has_study = true;
        if (t == "care") has_care = true;
        if (t == "updat") has_updat = true;
    }

    assert(has_run);
    assert(has_study);
    assert(has_care);
    assert(has_updat);

    std::cout << "test_tokenizer_integration passed\n";
}

int main() {
    test_individual_stems();
    test_priority_rules();
    test_no_overstemming();
    test_tokenizer_integration();

    std::cout << "ALL STEMMER TESTS PASSED\n";
}
