#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../common/mmap_file.hpp"
#include "../common/result.hpp"

struct Document {
    uint32_t doc_id;

    std::string url;
    std::string title;
    std::string text;
};

class DocStoreWriter {
public:
    void add(const Document& doc);

    Result<void> flush(const std::string& path);

private:
    std::vector<Document> docs_;
};

class DocStoreReader {
public:
    explicit DocStoreReader(const std::string& path);

    Result<Document> get(uint32_t doc_id) const;

    uint32_t doc_count() const {
        return count_;
    }

private:
    bool has_bytes(const uint8_t* ptr, size_t n) const;

private:
    MmapFile file_;

    const uint64_t	*offsets_ = nullptr;
    uint32_t		count_ = 0;
};
