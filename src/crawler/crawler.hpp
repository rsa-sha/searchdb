#pragma once

#include "../common/thread_pool.hpp"
#include "http_client.hpp"
#include "robots.hpp"
#include "url_frontier.hpp"
#include "url_utils.hpp"

#include <chrono>
#include <mutex>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>


struct CrawlConfig {
	std::vector<std::string> seed_urls;
	std::string output_dir;
	size_t max_pages = 100;
	size_t num_threads = 8;
	size_t max_depth = 5;					// BFS depth limit
};


class Crawler {
public:
	explicit Crawler(CrawlConfig config);

	// loop of crawler pipeline
	void run();

private:
	CrawlConfig		config_;
	URLFrontier		frontier_;
	RobotsChecker	robots_;
	ThreadPool		pool_;
	
	std::atomic<uint32_t> pages_crawled_{0};
	std::atomic<uint32_t> next_doc_id_{0};


	void crawl_worker_();
	// For URL extraction from fetched HTML page
	std::vector<std::string> extract_links_(const std::string &html, const std::string &base_url_);
};


