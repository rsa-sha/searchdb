#pragma once


#include <string>
#include <vector>
#include <unordered_set>

class Tokenizer {
public:
    Tokenizer(); // loads stop words

    // Tokenize raw text into normalized terms.
    std::vector<std::string> tokenize(std::string_view text) const;

    // Tokenize a query string (same pipeline, for consistency).
    std::vector<std::string> tokenize_query(std::string_view query) const;

private:
    std::unordered_set<std::string> stop_words_;

    // Split text on non-alphanumeric characters.
    std::vector<std::string_view> split_(std::string_view text) const;

    // Lowercase a string (in-place is fine).
    static std::string to_lower_(std::string_view s);

    bool is_stop_word_(std::string_view word) const;
};
