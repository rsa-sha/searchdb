#include "bm25_scorer.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <unordered_map>


BM25Scorer::BM25Scorer( uint32_t doc_count, double avgdl, const DocLengthReader* doc_lengths, double k1, double b) {
	avgdl_ = avgdl;
	N_ = doc_count;
	doc_lengths_reader_ = doc_lengths;

	k1_ = k1;
	b_ = b;
}


double BM25Scorer::idf (uint32_t doc_freq) const {
/*
 t		=> term being queried
 N_		=> No of docs
 df(t)	=> No of docs containing the term 't'
 ln		=> natural log
 IDF(t) = ln( 1 + ( (N_ - df(t) + 0.5) / ( df(t) + 0.5 ) ) )
 */
	return std::log(1.0 + (N_ - doc_freq + 0.5)/(doc_freq + 0.5));
}

// TF normalization
double BM25Scorer::score_term(uint32_t term_freq, uint32_t doc_length) const {
	double tf = static_cast<double>(term_freq);
	double dl = static_cast<double>(doc_length);

	double numerator = tf * (k1_ + 1.0);
	double denominator = tf + k1_*(1.0 - b_ + (b_ * (dl /avgdl_)));

	return numerator / denominator;
}


// Legacy query using InvertedIndexBuilder (used by tests)
std::vector<ScoredDoc> BM25Scorer::query (const std::vector<std::string> &query_terms, const InvertedIndexBuilder &index, size_t top_k) const {
	std::unordered_map <uint32_t, double> scores;

	const auto &inverted = index.index();

	for (const auto &term: query_terms) {
		auto it = inverted.find(term);

		if (it == inverted.end())
			continue;

		const auto &postings = it->second;
		if (postings.empty())
			continue;
		uint32_t doc_freq = static_cast<uint32_t>(postings.size());

		double idf_val = idf(doc_freq);
		std::cout << term
          << " df=" << doc_freq
          << " idf=" << idf_val << "\n";
		for (const auto &posting: postings) {
			uint32_t doc_id = posting.doc_id;
			double tf_score = score_term(posting.term_freq, doc_lengths_reader_->get(doc_id));

			scores[doc_id] += idf_val * tf_score;
		}
	}

	std::vector<ScoredDoc> results;
	results.reserve(scores.size());
	for(const auto &[doc_id, score]: scores)
		results.push_back({doc_id, score});

	// Sorting top-k results
	auto sort_results = [](const ScoredDoc& a,
                  const ScoredDoc& b) {
        return a.score > b.score;
    };

    if (results.size() > top_k) {
        std::partial_sort(results.begin(), results.begin() + top_k, results.end(), sort_results);
        results.resize(top_k);
    } else {
        std::sort(results.begin(), results.end(),sort_results);
	}

	for (auto &r : results) {
		std::cerr << "DEBUG final score doc=" << r.doc_id
				  << " score=" << r.score << "\n";
	}

	return results;
}


// Production query using mmap-based InvertedIndex (zero-copy)
std::vector<ScoredDoc> BM25Scorer::query (const std::vector<std::string> &query_terms, const InvertedIndex &index, size_t top_k) const {
	std::unordered_map <uint32_t, double> scores;

	for (const auto &term: query_terms) {
		PostingList pl = index.find(term);

		if (pl.empty())
			continue;

		uint32_t doc_freq = pl.size();

		double idf_val = idf(doc_freq);
		std::cout << term
          << " df=" << doc_freq
          << " idf=" << idf_val << "\n";

		for (const auto &posting: pl) {
			uint32_t doc_id = posting.doc_id;
			double tf_score = score_term(posting.term_freq, doc_lengths_reader_->get(doc_id));

			scores[doc_id] += idf_val * tf_score;
		}
	}

	std::vector<ScoredDoc> results;
	results.reserve(scores.size());
	for(const auto &[doc_id, score]: scores)
		results.push_back({doc_id, score});

	auto sort_results = [](const ScoredDoc& a,
                  const ScoredDoc& b) {
        return a.score > b.score;
    };

    if (results.size() > top_k) {
        std::partial_sort(results.begin(), results.begin() + top_k, results.end(), sort_results);
        results.resize(top_k);
    } else {
        std::sort(results.begin(), results.end(),sort_results);
	}

	for (auto &r : results) {
		std::cerr << "DEBUG final score doc=" << r.doc_id
				  << " score=" << r.score << "\n";
	}

	return results;
}
