#include "doc_store.hpp"

#include <cstring>
#include <fstream>
#include <stdexcept>


template<typename T>
void write_value(std::ofstream &out, const T &value) {
    out.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

template<typename T>
T read_value(const uint8_t *ptr) {
    T value;
    std::memcpy(&value, ptr, sizeof(T));
    return value;
}

uint64_t serialized_size(const Document &doc) {
    return
        sizeof(uint16_t) + doc.url.size() +
        sizeof(uint16_t) + doc.title.size() +
        sizeof(uint32_t) + doc.text.size();
}


void DocStoreWriter::add(const Document& doc) {
    docs_.push_back(doc);
}

Result<void> DocStoreWriter::flush(const std::string& path) {
	std::ofstream out(path, std::ios::binary);
	if (!out)
		return std::unexpected(SearchError::NotFound);
	// HEADER [doc count]
	const uint32_t count = static_cast<uint32_t>(docs_.size());
	// [u32 doc_count]
	write_value(out, count);

	// offsets [starting positions of different docs in bin file]
	std::vector<uint64_t> offsets;
	offsets.reserve(count);
	uint64_t current_offset = sizeof(uint32_t) + sizeof(uint64_t) * count;
	for(const auto &doc: docs_) {
		offsets.push_back(current_offset);
		current_offset += serialized_size(doc);
	}
	// Writing offsets to the file
	for (uint64_t offset: offsets) {
		write_value(out, offset);
	}
	// DOCUMENT Blobs
	for (const auto &doc: docs_) {
		// Writing URL of file
		uint16_t url_len = static_cast<uint16_t>(doc.url.size());
		write_value(out, url_len);
		out.write(doc.url.data(), url_len);
		// Writing title
		uint16_t title_len = static_cast<uint16_t>(doc.title.size());
		write_value(out, title_len);
		out.write(doc.title.data(), title_len);
		// write text
		uint32_t text_len = static_cast<uint32_t>(doc.text.size());
		write_value(out, text_len);
		out.write(doc.text.data(), text_len);
	}
	out.flush();
	if (!out)
		return std::unexpected(SearchError::IoError);
	return {};
}


DocStoreReader::DocStoreReader(const std::string& path)
    : file_(path)
{
	if (file_.size() < sizeof(uint32_t)) {
		throw std::runtime_error("Doc store too small");
	}

	const uint8_t *base = file_.data();
	count_ = read_value<uint32_t>(base);

	size_t required = sizeof(uint32_t) + sizeof(uint64_t) * count_;

	if (file_.size() < required) {
		throw std::runtime_error("Doc store is corrupted");
	}

	offsets_ = reinterpret_cast<const uint64_t*>(base + sizeof(uint32_t));
}

Result<Document> DocStoreReader::get(uint32_t doc_id) const {
    if (doc_id >= count_)
        return std::unexpected(SearchError::NotFound);

    uint64_t offset = offsets_[doc_id];

    if (offset >= file_.size())
        return std::unexpected(SearchError::Corrupt);

    const uint8_t *ptr = file_.data() + offset;

    Document doc;
    doc.doc_id = doc_id;

    // URL
    if (!has_bytes(ptr, sizeof(uint16_t)))
        return std::unexpected(SearchError::Corrupt);
    uint16_t url_len = read_value<uint16_t>(ptr);
    ptr += sizeof(uint16_t);

    if (!has_bytes(ptr, url_len))
        return std::unexpected(SearchError::Corrupt);
    doc.url.assign(reinterpret_cast<const char *>(ptr), url_len);
    ptr += url_len;

    // TITLE
    if (!has_bytes(ptr, sizeof(uint16_t)))
        return std::unexpected(SearchError::Corrupt);
    uint16_t title_len = read_value<uint16_t>(ptr);
    ptr += sizeof(uint16_t);
    if (!has_bytes(ptr, title_len))
        return std::unexpected(SearchError::Corrupt);
    doc.title.assign(reinterpret_cast<const char *>(ptr), title_len);
    ptr += title_len;

	// TEXT
    if (!has_bytes(ptr, sizeof(uint32_t)))
        return std::unexpected(SearchError::Corrupt);
    uint32_t text_len = read_value<uint32_t>(ptr);
    ptr += sizeof(uint32_t);
    if (!has_bytes(ptr, text_len))
        return std::unexpected(SearchError::Corrupt);
    doc.text.assign(reinterpret_cast<const char *>(ptr), text_len);
    return doc;
}

bool DocStoreReader::has_bytes(const uint8_t *ptr, size_t n) const {
    const uint8_t *begin = file_.data();
    const uint8_t *end = begin + file_.size();
    return ptr + n <= end;
}

