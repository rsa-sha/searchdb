/* ERROR Handling system */
#pragma once

#include <expected>
#include <string>


enum class SearchError {
	IoError,			// Disk Failure
	ParseError,			// Unable to format user req or fetched data
	NotFound,			// Nothing found at the url
	InvalidArgs
};

template <typename err_t>
using Result = std::expected<err_t, SearchError>;
