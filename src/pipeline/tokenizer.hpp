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

	// TEST ONLY - do not use in production pipeline
    std::string stem_public(std::string_view token) const;


private:
    std::unordered_set<std::string> stop_words_;

    // Split text on non-alphanumeric characters.
    std::vector<std::string_view> split_(std::string_view text) const;

    // Lowercase a string (in-place is fine).
    static std::string to_lower_(std::string_view s);

    bool is_stop_word_(std::string_view word) const;

	// Stemmer
	std::string stem_(std::string_view token) const;
	
	static bool ends_with_(std::string_view s, std::string_view suf);
	
	static void collapse_double_(std::string &s);
};
