#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>


class RobotsChecker {
public:
	RobotsChecker() = default;

	// Checks if URL is allowed. Fetches + caches robots.txt per domain
	bool is_allowed(const std::string &url);

	// Get crawl delay for a domain (Crawl-delay| deafult value)
	std::chrono::milliseconds crawl_delay(const std::string &domain);

private:
	// This struct is prepared from the fetched `robots.txt` data of a domain
	struct RobotsRules {
		std::vector<std::string> disallowed_paths;
		std::chrono::milliseconds crawl_delay = std::chrono::milliseconds(1000);
	};

	// Robots.txt cache map of multiple domains
	std::unordered_map<std::string, RobotsRules> robots_cache_;
	std::mutex mu_;

	// Fetches robots.txt from a domain, parses it to the RobotsRules structure format
	RobotsRules fetch_and_parse_(const std::string &domain);

	static std::string extract_path(const std::string &url);

	static std::string trim(const std::string &s);

	static bool starts_with(const std::string &str, const std::string &prefix);
};

