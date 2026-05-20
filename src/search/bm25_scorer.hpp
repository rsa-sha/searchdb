#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../index/doc_lengths.hpp"
#include "../index/inverted_index.hpp"


struct ScoredDoc {
	uint32_t	doc_id;
	double		score;
};



class BM25Scorer {
public:
	BM25Scorer(uint32_t doc_count, double avgdl, const DocLengthReader *doc_lengths, double k1 = 1.2, double b = 0.75);

	double idf(uint32_t doc_freq) const;

	double score_term(uint32_t term_freq, uint32_t doc_length) const;

	std::vector<ScoredDoc> query (const std::vector<std::string> &query_terms, const InvertedIndexBuilder &index, size_t top_k = 10) const;

private:
	uint32_t		N_;
	double			avgdl_;
	const DocLengthReader *doc_lengths_reader_;


	double k1_;
	double b_;
};
