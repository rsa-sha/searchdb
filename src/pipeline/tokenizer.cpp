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

static inline bool is_alnum_(char c) {
    return std::isalnum(static_cast<unsigned char>(c));
}

// PRIVATE Methods

// Split text on non-alphanumeric characters.
std::vector<std::string_view> Tokenizer::split_(std::string_view text) const {
	std::vector<std::string_view> tokens;
	size_t start = 0;
	size_t end = text.size();
	while (start < end) {
		// skip non-alphanumeric values
		while ((start < end) && !is_alnum_(text[start]))
			start++;
		if (start >= end)
			break;
		// Find end of alphaunmeric run
		size_t last = start;
		while ((last < end) && is_alnum_(text[last]))
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

bool Tokenizer::ends_with_(std::string_view s, std::string_view suf) {
    return s.size() >= suf.size() &&
           s.substr(s.size() - suf.size()) == suf;
}

void Tokenizer::collapse_double_(std::string &s) {
    if (s.size() < 2)
        return;
    char last = s.back();
    if (last != s[s.size() - 2])
        return;
    // preserve lexical doubles
    if (last == 's' || last == 'l')
        return;

    s.pop_back();
}

std::string Tokenizer::stem_(std::string_view token) const {
    std::string w(token);

    auto strip = [&](size_t n) {
        return w.substr(0, w.size() - n);
    };

    bool stripped = false;

    // STEP 1: adverb / adjective cleanup first
    if (ends_with_(w, "ingly") && w.size() > 6) {
        w = strip(5);
        stripped = true;
    } else if (ends_with_(w, "ing") && w.size() > 5) {
        w = strip(3);
        stripped = true;
    } else if (ends_with_(w, "ed") && w.size() > 4) {
        w = strip(2);
        stripped = true;
    }

    // Fix doubled consonants after stripping
    if (stripped) {
        collapse_double_(w);
    }

    // STEP 2: derivational suffixes
    if (ends_with_(w, "fully") && w.size() > 6) {
        w = strip(5);
    } else if (ends_with_(w, "ful") && w.size() > 5) {
        w = strip(3);
    }

    if (ends_with_(w, "ness") && w.size() > 5) {
        w = strip(4);
    }

    if (ends_with_(w, "ment") && w.size() > 5) {
        w = strip(4);
    }

    // STEP 3: plural rules
    if (ends_with_(w, "ies") && w.size() > 4) {
        w = strip(3) + "y";
    } else if (ends_with_(w, "es") && w.size() > 4) {
        w = strip(2);
    } else if (ends_with_(w, "s") && !ends_with_(w, "ss") && w.size() > 3) {
        w = strip(1);
    }

    return w;
}


// PUBLIC Methods

std::vector<std::string> Tokenizer::tokenize(std::string_view text) const {
	std::vector<std::string> tokens;
	for (auto token_view: split_(text)) {
		std::string token = to_lower_(token_view);
		if (is_stop_word_(token))
			continue;
		token = stem_(token);
		tokens.push_back(std::move(token));
	}
	return tokens;
}

std::vector<std::string> Tokenizer::tokenize_query(std::string_view query) const {
    return tokenize(query);
}

std::string Tokenizer::stem_public(std::string_view token) const {
    return stem_(token);
}
