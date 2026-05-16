/* ERROR Handling system */
#pragma once

#include <expected>
#include <string>


enum class SearchError {
	IoError,			// Disk Failure
	ParseError,			// Unable to format user req or fetched data
	NotFound,			// Nothing found at the url
	InvalidArgs,
	MemError,			// To be sent when unable to allocate mem or create variable
	NetworkError,
	Timeout,
	HttpError,
	Corrupt				// File being read is corrupted
};

inline const char* to_string(SearchError err) {
    switch (err) {
        case SearchError::IoError: return "IoError";
        case SearchError::ParseError: return "ParseError";
        case SearchError::NotFound: return "NotFound";
        case SearchError::InvalidArgs: return "InvalidArgs";
        case SearchError::MemError: return "MemoryError";
        case SearchError::NetworkError: return "NetworkError";
        case SearchError::Timeout: return "Timeout";
        case SearchError::HttpError: return "HttpError";
        default: return "UnknownError";
    }
}

template <typename err_t>
using Result = std::expected<err_t, SearchError>;
