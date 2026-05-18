#include "inverted_index.hpp"


#include <algorithm>
#include <unordered_map>

void InvertedIndexBuilder::add_document(uint32_t doc_id, const std::vector<std::string> &tokens) {
    // Ensure vector is large enough
    if (doc_id >= doc_lengths_.size()) {
        doc_lengths_.resize(doc_id + 1, 0);
    }

    // Count term frequencies locally
    std::unordered_map<std::string, uint32_t> term_freqs;

    for (const auto &token : tokens) {
        term_freqs[token]++;
    }

    // Add postings
    for (const auto &[term, freq] : term_freqs) {
        index_[term].push_back(
            Posting{
                .doc_id = doc_id,
                .term_freq = freq
            }
        );
    }
    // Track document stats
    doc_lengths_[doc_id] = tokens.size();
    doc_count_++;
    total_tokens_ += tokens.size();
}

void InvertedIndexBuilder::finalize() {
    for (auto &[term, postings] : index_) {
        std::sort( postings.begin(), postings.end(),
				[](const Posting &a, const Posting &b) {
					return a.doc_id < b.doc_id;
				}
		);
    }
}

const std::unordered_map<std::string, std::vector<Posting>> & InvertedIndexBuilder::index() const {
    return index_;
}

uint32_t InvertedIndexBuilder::doc_count() const {
    return doc_count_;
}

double InvertedIndexBuilder::avg_doc_length() const {
    if (doc_count_ == 0)
		return 0.0;
    return static_cast<double>(total_tokens_) / static_cast<double>(doc_count_);
}
