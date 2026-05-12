#include "robots.hpp"
#include "url_utils.hpp"
#include "http_client.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>


bool RobotsChecker::is_allowed (const std::string &url) {
	// Normalize the URL and extract domain
	auto normalized_url = normalize_url(url);
	if (normalized_url.empty())
		return false;

	auto domain = extract_domain(normalized_url);

	RobotsRules rules;
	bool need_to_fetch = false;
	{
		std::lock_guard lock(mu_);

		auto it = robots_cache_.find(domain);

		if (it == robots_cache_.end()) {
			// Fetching outside of lock by setting  need_to_fetch as true
			// rules = fetch_and_parse_(domain);
			robots_cache_[domain] = RobotsRules{};
			need_to_fetch = true;
		} else
			rules = it->second;
	}
	if (need_to_fetch) {
		rules = fetch_and_parse_(domain);
		std::lock_guard lock(mu_);
		robots_cache_[domain] = rules;
	}
	// Now we check for specific path related Allow/Disallow settings
	const std::string path = extract_path(normalized_url);

	for (const auto& disallowed : rules.disallowed_paths) {
		if (disallowed.empty())
			continue;
		if (starts_with(path, disallowed))
			return false;
	}
	return true;
}

std::chrono::milliseconds RobotsChecker::crawl_delay(const std::string &domain) {

    std::lock_guard<std::mutex> lock(mu_);

    auto it = robots_cache_.find(domain);
    if (it == robots_cache_.end()) {
        RobotsRules rules = fetch_and_parse_(domain);
        robots_cache_[domain] = rules;
        return rules.crawl_delay;
    }
    return it->second.crawl_delay;
}

RobotsChecker::RobotsRules RobotsChecker::fetch_and_parse_(const std::string &domain) {
	std::cout << "[robots] fetching robots.txt for " << domain << std::endl;
    RobotsRules rules;
    const std::string robots_url = "https://" + domain + "/robots.txt";

    auto response_result = fetch(robots_url);

	// [robots unavailable, got errror while fetching]=> allow crawl
	if (!response_result.has_value())
		return rules;

    const auto& response = response_result.value();
    if (response.status_code != 200) {
        return rules;
    }

    std::istringstream stream(response.body);
    std::string line;
    bool in_user_agent_block = false;

    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty())
            continue;

        // Remove comments
        auto comment_pos = line.find('#');
        if (comment_pos != std::string::npos)
            line = trim(line.substr(0, comment_pos));

        if (line.empty())
            continue;

        auto colon_pos = line.find(':');
        if (colon_pos == std::string::npos)
            continue;

        std::string key = trim(line.substr(0, colon_pos));
        std::string value = trim(line.substr(colon_pos + 1));

        // lowercase key
        std::transform(
				key.begin(), key.end(), key.begin(),
				[](unsigned char c) {
				return static_cast<char>(std::tolower(c));
				});

        if (key == "user-agent") {
			std::string ua = value;
            std::transform(
					ua.begin(), ua.end(), ua.begin(),
					[](unsigned char c) {
					return static_cast<char>(std::tolower(c));
					});
            in_user_agent_block = (ua == "*");
        }

        else if (in_user_agent_block && key == "disallow")
            rules.disallowed_paths.push_back(value);
        else if (in_user_agent_block && key == "crawl-delay") {
            try {
                int seconds = std::stoi(value);
                if (seconds > 0) {
                    rules.crawl_delay =
                        std::chrono::milliseconds(seconds * 1000);
                }
            } catch (...) {
                // ignore malformed crawl-delay
            }
        }
    }
    return rules;
}

std::string RobotsChecker::extract_path(const std::string &url) {
    auto scheme_pos = url.find("://");
    if (scheme_pos == std::string::npos)
        return "/";

    auto path_pos = url.find('/', scheme_pos + 3);
    if (path_pos == std::string::npos)
        return "/";
    return url.substr(path_pos);
}

std::string RobotsChecker::trim(const std::string& s) {
    size_t start = 0;
    while ((start < s.size()) && std::isspace(static_cast<unsigned char>(s[start]))) {
        start++;
    }

    size_t end = s.size();
    while ((end > start) && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        end--;
    }
    return s.substr(start, end - start);
}

bool RobotsChecker::starts_with(const std::string &str, const std::string& prefix) {
	return str.rfind(prefix, 0) == 0;
}
