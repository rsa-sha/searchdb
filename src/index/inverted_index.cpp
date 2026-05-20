#include "inverted_index.hpp"


#include <algorithm>
#include <fstream>
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
		total_postings_++;
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

const std::vector<uint32_t> & InvertedIndexBuilder::doc_lengths() const {
    return doc_lengths_;
}

double InvertedIndexBuilder::avg_doc_length() const {
    if (doc_count_ == 0)
		return 0.0;
    return static_cast<double>(total_tokens_) / static_cast<double>(doc_count_);
}

// STAT methods
uint64_t InvertedIndexBuilder::total_postings() const {
    return total_postings_;
}

size_t InvertedIndexBuilder::vocabulary_size() const {
    return index_.size();
}

std::pair<std::string, size_t>
InvertedIndexBuilder::longest_posting_list() const {
    std::string best_term;
    size_t best_size = 0;

    for (const auto &[term, postings] : index_) {
        if (postings.size() > best_size) {
            best_size = postings.size();
            best_term = term;
        }
    }

    return {best_term, best_size};
}


void serialize_index(const InvertedIndexBuilder &builder, const std::string &path) {
    std::ofstream out(path, std::ios::binary);
    const auto &index = builder.index();

	// =========================
    // 1. WRITE METADATA HEADER
    // =========================
    uint32_t doc_count = builder.doc_count();
    uint32_t total_tokens = builder.total_tokens();
    uint64_t total_postings = builder.total_postings();

    out.write(reinterpret_cast<const char*>(&doc_count), sizeof(doc_count));
    out.write(reinterpret_cast<const char*>(&total_tokens), sizeof(total_tokens));
    out.write(reinterpret_cast<const char*>(&total_postings), sizeof(total_postings));

	// =========================
    // 2. WRITE INVERTED INDEX
    // =========================
	uint64_t num_terms = index.size();
    out.write(reinterpret_cast<const char *>(&num_terms), sizeof(num_terms));

    for (const auto& [term, postings] : index) {
        uint32_t term_len = term.size();
        out.write(reinterpret_cast<const char *>(&term_len), sizeof(term_len));
        out.write(term.data(), term_len);

        uint32_t posting_count = postings.size();
        out.write(reinterpret_cast<const char *>(&posting_count), sizeof(posting_count));
        for (const auto &p : postings) {
            out.write(reinterpret_cast<const char *>(&p.doc_id), sizeof(p.doc_id));
            out.write(reinterpret_cast<const char *>(&p.term_freq), sizeof(p.term_freq));
        }
    }
}

InvertedIndexBuilder load_index(const std::string &path){
    InvertedIndexBuilder builder;

    std::ifstream in(path, std::ios::binary);

    // =========================
    // 1. READ METADATA HEADER
    // =========================
    uint32_t doc_count;
    uint32_t total_tokens;
    uint64_t total_postings;

    in.read(reinterpret_cast<char*>(&doc_count), sizeof(doc_count));
    in.read(reinterpret_cast<char*>(&total_tokens), sizeof(total_tokens));
    in.read(reinterpret_cast<char*>(&total_postings), sizeof(total_postings));

    builder.doc_count_ = doc_count;
    builder.total_tokens_ = total_tokens;
    builder.total_postings_ = total_postings;

    // =========================
    // 2. READ INVERTED INDEX
    // =========================
    uint64_t num_terms;
    in.read(reinterpret_cast<char *>(&num_terms), sizeof(num_terms));

    for (uint64_t i = 0; i < num_terms; ++i) {

        uint32_t term_len;
        in.read(reinterpret_cast<char *>(&term_len), sizeof(term_len));

        std::string term(term_len, '\0');
        in.read(term.data(), term_len);

        uint32_t posting_count;
        in.read(reinterpret_cast<char *>(&posting_count), sizeof(posting_count));

        std::vector<Posting> postings;
        postings.reserve(posting_count);

        for (uint32_t j = 0; j < posting_count; ++j) {
            Posting p;
            in.read(reinterpret_cast<char *>(&p.doc_id), sizeof(p.doc_id));
            in.read(reinterpret_cast<char *>(&p.term_freq), sizeof(p.term_freq));
            postings.push_back(p);
        }

        builder.index_.emplace(term, std::move(postings));
    }

    builder.finalize();
    return builder;
}
