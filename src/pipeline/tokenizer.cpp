#include "tokenizer.hpp"


// Initialize stop_words_
Tokenizer::Tokenizer()
{
    static const std::string STOP_WORDS[] = {
		"a", "an", "and", "are", "as", "at", "be", "by", "for",
		"from", "in", "is", "it", "of", "on", "the", "to", "with",

        "about", "after", "all", "also", "am", "any","been", "before",
		"being", "but", "can", "could", "did", "do", "does", "doing",
		"down", "during", "each", "few", "had", "have", "having", "he",
        "her", "here", "hers", "him", "himself", "his", "how", "i",
		"if", "into", "me", "more", "most", "my", "no", "not", "now",
		"or", "other", "our", "out", "over", "own", "same", "she",
		"should","so", "some", "such", "than", "that", "their", "them", 
		"then", "there", "these", "they", "this", "those", "through",
		"under", "up", "very", "was", "we", "were", "what", "when",
		"where", "which", "who", "why", "will", "you", "your"
    };

    stop_words_.reserve(std::size(STOP_WORDS));

    for (auto w : STOP_WORDS) {
        stop_words_.insert(w);
    }
}

// PRIVATE Methods

// Split text on non-alphanumeric characters.
std::vector<std::string_view> Tokenizer::split_(std::string_view text) const {
	std::vector<std::string_view> tokens;
	size_t start = 0;
	size_t end = text.size();
	while (start < end) {
		// skip non-alphanumeric values
		while ((start < end) && !isalnum(text[start]))
			start++;
		if (start >= end)
			break;
		// Find end of alphaunmeric run
		size_t last = start;
		while ((last < end) && isalnum(text[last]))
			last++;
		// skip tokens with length less than 2
		if (last - start >= 2)
			tokens.emplace_back(text.substr(start, last - start));
		start = last;
	}
	return tokens;
}

bool Tokenizer::is_stop_word_(std::string_view word) const {
	return stop_words_.find(std::string(word)) != stop_words_.end();
}

std::string Tokenizer::to_lower_(std::string_view s) {
	std::string result;
	result.reserve(s.size());
	for (char c:s) {
		if (c >= 'A' && c <= 'Z')
			result += (c +32);
		else
			result += c;
	}
	return result;
}


// PUBLIC Methods

std::vector<std::string> Tokenizer::tokenize(std::string_view text) const {
	std::vector<std::string> tokens;
	for (auto token_view: split_(text)) {
		std::string token = to_lower_(token_view);
		if (!is_stop_word_(token))
			tokens.push_back(std::move(token));
	}
	return tokens;
}

std::vector<std::string> Tokenizer::tokenize_query(std::string_view query) const {
    return tokenize(query);
}
