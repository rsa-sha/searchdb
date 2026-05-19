#include "doc_lengths.hpp"



Result<void> DocLengthWriter::write(const std::string &path, const std::vector<uint32_t> &doc_lengths) {
	std::ofstream out_file(path, std::ios::binary);

	if (!out_file)
		return std::unexpected(SearchError::NotFound);

	for (uint32_t length: doc_lengths)
		out_file.write(reinterpret_cast<const char *>(&length), sizeof(uint32_t));

	out_file.flush();

	if (!out_file)
		return std::unexpected(SearchError::IoError);

	return {};
}


DocLengthReader::DocLengthReader(const std::string& path)
    : file_(path)
{
    if (file_.size() % sizeof(uint32_t) != 0)
        throw std::runtime_error("Corrupt doc_lengths.bin");

    count_ = file_.size() / sizeof(uint32_t);

    lengths_ = reinterpret_cast<const uint32_t *>(file_.data());
}

uint32_t DocLengthReader::get(uint32_t doc_id) const {
    if (doc_id >= count_)
        throw std::out_of_range("Invalid doc_id");

    return lengths_[doc_id];
}
