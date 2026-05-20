#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct Posting {
	uint32_t	doc_id;
	uint32_t	term_freq;
};


class InvertedIndexBuilder {
public:
	// Process one document and add postings
	void add_document(uint32_t doc_id, const std::vector<std::string> &tokens);

	// Sort posting lists by doc_id
    void finalize();

    // Accessors
    const std::unordered_map<std::string,std::vector<Posting>> &index() const;

    uint32_t doc_count() const;

    double avg_doc_length() const;

	const std::vector<uint32_t> &doc_lengths() const;
	uint64_t total_postings() const;
	size_t vocabulary_size() const;
	std::pair<std::string, size_t> longest_posting_list() const;

	uint32_t total_tokens() const { return total_tokens_; }
	friend InvertedIndexBuilder load_index(const std::string &path);

private:
	std::unordered_map<std::string, std::vector<Posting>> index_;
	std::vector<uint32_t> doc_lengths_;

	uint32_t doc_count_ = 0;
	uint32_t total_tokens_ = 0;
	uint64_t total_postings_ = 0;
};


void serialize_index(const InvertedIndexBuilder &builder, const std::string &path);
InvertedIndexBuilder load_index(const std::string &path);
