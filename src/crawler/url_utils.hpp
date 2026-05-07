#pragma once

#include <string>
#include <optional>


// Normalize URL for dedup. Returns empty string if URL is invalid.
std::string normalize_url(const std::string &url);

// Extract domain (host) from URL. E.g., "https://en.wikipedia.org/wiki/X" → "en.wikipedia.org"
std::string extract_domain(const std::string &url);

// Resolve relative URL against a base URL.
std::string resolve_url(const std::string &base, const std::string &relative);
