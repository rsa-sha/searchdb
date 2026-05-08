#include "url_frontier.hpp"
#include "url_utils.hpp"

#include <thread>

URLFrontier::URLFrontier (std::chrono::milliseconds politeness_delay):delay_(politeness_delay){}

void URLFrontier::push (const std::string &url) {
	auto normalized_url = normalize_url(url);

	if (normalized_url.empty())
		return;

	auto domain = extract_domain(normalized_url);

	std::lock_guard lock(mu_);

	if (url_set_.contains(normalized_url))
		return;

	url_set_.insert(normalized_url);
	FrontierItem item;
	item.url = normalized_url;
	item.domain = domain;
	queue_.push_back(item);
	cv_.notify_one();
}


void URLFrontier::push_multiple (const std::vector<std::string> &urls) {
	for (const auto &url:urls)
		URLFrontier::push(url);
}

size_t URLFrontier::pending() const {
	std::lock_guard lock(mu_);
	return queue_.size();
}

size_t URLFrontier::seen() const {
	std::lock_guard lock(mu_);
	return url_set_.size();
}

void URLFrontier::mark_done() {
	{
		std::lock_guard lock(mu_);
		done_ = true;
	}
	cv_.notify_all();
}

std::optional<std::string> URLFrontier::pop() {
    std::unique_lock lock(mu_);

    while (true) {
        if (done_ && queue_.empty())
            return std::nullopt;

        auto now = std::chrono::steady_clock::now();
        std::optional<size_t> ready_index;
        std::chrono::steady_clock::time_point earliest = now + std::chrono::hours(24);

        for (size_t i = 0; i < queue_.size(); ++i) {
            const auto &item = queue_[i];
            auto it = domain_next_visit_time_.find(item.domain);
            if (it == domain_next_visit_time_.end() || now >= it->second) {
                ready_index = i;
                break;
            }
            if (it->second < earliest)
                earliest = it->second;
        }

        if (ready_index) {
            auto item = queue_[*ready_index];
            queue_.erase(queue_.begin() + *ready_index);
            domain_next_visit_time_[item.domain] = now + delay_;
            return item.url;
        }

        // avoid long blocking wait_until
        if (queue_.empty()) {
            cv_.wait(lock, [this] {
                return done_ || !queue_.empty();
            });
        } else {
            // wait only briefly, then re-check logic
            cv_.wait_for(lock, std::chrono::milliseconds(10));
        }
    }
}
