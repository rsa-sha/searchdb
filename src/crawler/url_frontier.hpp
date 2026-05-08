#pragma once

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


class URLFrontier {
public:
	explicit URLFrontier (
			// politeness_delay is the min wait b/w requests to the same host/domain
			std::chrono::milliseconds politeness_delay = std::chrono::milliseconds(100)
			);

	// Add URL, Normalize + dedup [Thread-Safe]
	void push(const std::string &url);

	// Bulk add for seeding
	void push_multiple(const std::vector<std::string> &urls);

	// Get next URL that respects politeness. Blocks briefly if nothing is ready
	// Returns nullopt if frontier is empty AND done flag is set
	std::optional<std::string> pop();

	// Signal that no more URLs will be added
	void mark_done();

	size_t pending() const;		// URLs in queue
	size_t seen() const;		// Total number of unique URLs ever added;

private:
	struct FrontierItem {
		std::string url;
		std::string domain;
	};
	std::deque<FrontierItem> queue_;

	std::unordered_set<std::string> url_set_;
	std::unordered_map<std::string, std::chrono::steady_clock::time_point> domain_next_visit_time_;
	

	mutable std::mutex mu_;
	std::condition_variable cv_;
	std::chrono::milliseconds delay_;
	bool done_ = false;
};
