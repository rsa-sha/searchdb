/* RAW Data Access Layer */
#pragma once

#include <cstdint>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

class MmapFile {
public:
	MmapFile(const std::string &path) {
		fd_ = open(path.c_str(), O_RDONLY);
		if (fd_ < 0 )
			throw std::runtime_error("open failed");

		struct stat st;
		if (fstat(fd_, &st) < 0)
			throw std::runtime_error("fstat failed");
		size_ = st.st_size;

		data_ = static_cast<uint8_t*>(
				mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, 0)
				);

		if (data_ == MAP_FAILED)
			throw std::runtime_error("mmap failed");
	}

	// no copying
	MmapFile(const MmapFile &) = delete;
	MmapFile & operator=(const MmapFile &) = delete;

	// movable
	MmapFile(MmapFile &&other) noexcept {
		*this = std::move(other);
	}
	MmapFile& operator=(MmapFile &&other) noexcept {
		if (this == &other)
			return *this;

		cleanup();

		fd_ = other.fd_;
		data_ = other.data_;
		size_ = other.size_;

		other.fd_ = -1;
		other.data_ = nullptr;
		other.size_ = 0;

		return *this;
	}

	const uint8_t* data() const {return data_;}
	size_t size() const {return size_;}

	~MmapFile() {
		if (data_)
			munmap(data_, size_);
		if (fd_ >= 0)
			close(fd_);
	}

private:
	void cleanup() {
        if (data_ && data_ != MAP_FAILED)
            munmap(data_, size_);

        if (fd_ >= 0)
            close(fd_);
    }

	int			fd_{-1};
	uint8_t*	data_{nullptr};
	size_t		size_{0};
};
