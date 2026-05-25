#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../common/mmap_file.hpp"
#include "../pipeline/tokenizer.hpp"

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


// ============================================================
// mmap-based read-only index (zero-copy, for query time)
// ============================================================

struct PostingList {
    const Posting* data;
    uint32_t count;

    const Posting* begin() const { return data; }
    const Posting* end()   const { return data + count; }
    uint32_t size()        const { return count; }
    bool empty()           const { return count == 0; }
};

struct TermEntry {
    uint32_t term_offset;    // byte offset into terms blob
    uint32_t term_length;    // length of term string
    uint32_t postings_index; // index into postings blob (in Posting units)
    uint32_t posting_count;  // number of postings for this term
};

struct IndexHeader {
    uint32_t magic;            // 0x49445832 ("IDX2")
    uint32_t doc_count;
    uint32_t total_tokens;
    uint32_t num_terms;
    uint64_t total_postings;
    uint64_t postings_blob_off;
};

static_assert(sizeof(IndexHeader) == 32, "IndexHeader must be 32 bytes");
static_assert(sizeof(TermEntry) == 16, "TermEntry must be 16 bytes");
static_assert(sizeof(Posting) == 8, "Posting must be 8 bytes (no padding)");

class InvertedIndex {
public:
    explicit InvertedIndex(const std::string& path);

    PostingList find(std::string_view term) const;

    uint32_t doc_count()       const;
    double   avg_doc_length()  const;
    uint32_t vocabulary_size() const;

private:
    MmapFile            file_;
    const IndexHeader*  header_;
    const TermEntry*    term_dir_;
    const char*         terms_blob_;
    const Posting*      postings_blob_;
};

void serialize_index_v2(const InvertedIndexBuilder& builder, const std::string& path);
