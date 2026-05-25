#include "inverted_index.hpp"


#include <algorithm>
#include <cstring>
#include <fstream>
#include <stdexcept>
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
        out.write(reinterpret_cast<const char*>(postings.data()),
                  posting_count * sizeof(Posting));
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

    builder.index_.reserve(num_terms);

    for (uint64_t i = 0; i < num_terms; ++i) {

        uint32_t term_len;
        in.read(reinterpret_cast<char *>(&term_len), sizeof(term_len));

        std::string term(term_len, '\0');
        in.read(term.data(), term_len);

        uint32_t posting_count;
        in.read(reinterpret_cast<char *>(&posting_count), sizeof(posting_count));

        std::vector<Posting> postings(posting_count);
        in.read(reinterpret_cast<char*>(postings.data()),
                posting_count * sizeof(Posting));

        builder.index_.emplace(std::move(term), std::move(postings));
    }

    return builder;
}


// ============================================================
// v2 serializer: mmap-friendly format with sorted term directory
// ============================================================

void serialize_index_v2(const InvertedIndexBuilder& builder, const std::string& path) {
    const auto& index = builder.index();

    // 1. Collect and sort terms alphabetically
    std::vector<std::string> sorted_terms;
    sorted_terms.reserve(index.size());
    for (const auto& [term, _] : index)
        sorted_terms.push_back(term);
    std::sort(sorted_terms.begin(), sorted_terms.end());

    // 2. Build the terms blob and postings blob, tracking offsets
    std::vector<TermEntry> term_dir;
    term_dir.reserve(sorted_terms.size());

    std::vector<char> terms_blob;
    std::vector<Posting> postings_blob;

    for (const auto& term : sorted_terms) {
        const auto& postings = index.at(term);

        TermEntry entry;
        entry.term_offset    = static_cast<uint32_t>(terms_blob.size());
        entry.term_length    = static_cast<uint32_t>(term.size());
        entry.postings_index = static_cast<uint32_t>(postings_blob.size());
        entry.posting_count  = static_cast<uint32_t>(postings.size());
        term_dir.push_back(entry);

        // Append term string
        terms_blob.insert(terms_blob.end(), term.begin(), term.end());

        // Append postings
        postings_blob.insert(postings_blob.end(), postings.begin(), postings.end());
    }

    // 3. Compute layout offsets
    uint32_t num_terms = static_cast<uint32_t>(sorted_terms.size());
    uint64_t term_dir_off     = sizeof(IndexHeader);                             // 32
    uint64_t terms_blob_off   = term_dir_off + num_terms * sizeof(TermEntry);
    uint64_t postings_blob_off = terms_blob_off + terms_blob.size();

    // 4. Write everything
    std::ofstream out(path, std::ios::binary);
    if (!out)
        throw std::runtime_error("serialize_index_v2: failed to open " + path);

    // Header
    IndexHeader hdr;
    hdr.magic            = 0x49445832; // "IDX2"
    hdr.doc_count        = builder.doc_count();
    hdr.total_tokens     = builder.total_tokens();
    hdr.num_terms        = num_terms;
    hdr.total_postings   = builder.total_postings();
    hdr.postings_blob_off = postings_blob_off;

    out.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));

    // Term directory
    out.write(reinterpret_cast<const char*>(term_dir.data()),
              term_dir.size() * sizeof(TermEntry));

    // Terms blob
    out.write(terms_blob.data(), terms_blob.size());

    // Postings blob
    out.write(reinterpret_cast<const char*>(postings_blob.data()),
              postings_blob.size() * sizeof(Posting));

    out.flush();
    if (!out)
        throw std::runtime_error("serialize_index_v2: write failed for " + path);
}


// ============================================================
// InvertedIndex: mmap-based read-only index
// ============================================================

InvertedIndex::InvertedIndex(const std::string& path)
    : file_(path)
{
    if (file_.size() < sizeof(IndexHeader))
        throw std::runtime_error("InvertedIndex: file too small");

    const uint8_t* base = file_.data();
    header_ = reinterpret_cast<const IndexHeader*>(base);

    if (header_->magic != 0x49445832)
        throw std::runtime_error("InvertedIndex: bad magic (not v2 format)");

    uint32_t n = header_->num_terms;

    // Validate file has enough room for the term directory
    size_t term_dir_end = sizeof(IndexHeader) + n * sizeof(TermEntry);
    if (file_.size() < term_dir_end)
        throw std::runtime_error("InvertedIndex: truncated term directory");

    term_dir_ = reinterpret_cast<const TermEntry*>(base + sizeof(IndexHeader));

    // Terms blob starts right after the term directory
    terms_blob_ = reinterpret_cast<const char*>(base + term_dir_end);

    // Postings blob at the offset stored in header
    if (header_->postings_blob_off > file_.size())
        throw std::runtime_error("InvertedIndex: invalid postings offset");

    postings_blob_ = reinterpret_cast<const Posting*>(base + header_->postings_blob_off);
}

PostingList InvertedIndex::find(std::string_view term) const {
    uint32_t n = header_->num_terms;
    if (n == 0)
        return {nullptr, 0};

    // Binary search the sorted term directory
    int lo = 0;
    int hi = static_cast<int>(n) - 1;

    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        const TermEntry& e = term_dir_[mid];
        std::string_view entry_term(terms_blob_ + e.term_offset, e.term_length);

        int cmp = entry_term.compare(term);
        if (cmp == 0) {
            return {postings_blob_ + e.postings_index, e.posting_count};
        } else if (cmp < 0) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    return {nullptr, 0};
}

uint32_t InvertedIndex::doc_count() const {
    return header_->doc_count;
}

double InvertedIndex::avg_doc_length() const {
    if (header_->doc_count == 0)
        return 0.0;
    return static_cast<double>(header_->total_tokens) /
           static_cast<double>(header_->doc_count);
}

uint32_t InvertedIndex::vocabulary_size() const {
    return header_->num_terms;
}
