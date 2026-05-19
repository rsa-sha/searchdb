#pragma once
// Tiny helper for BM25 metadata

#include <fstream>
#include <string>
#include <vector>

#include "../common/result.hpp"
#include "../common/mmap_file.hpp"


class DocLengthWriter {
public:
	static Result<void> write(const std::string &path, const std::vector<uint32_t> &doc_lengths);
};


class DocLengthReader {
public:
	explicit DocLengthReader(const std::string &path);

	uint32_t get(uint32_t doc_id) const;

	uint32_t count() const{
		return count_;
	}

private:
	MmapFile file_;

	const uint32_t *lengths_ = nullptr;
	uint32_t count_ = 0;
};
