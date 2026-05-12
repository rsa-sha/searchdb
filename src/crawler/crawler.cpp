#include "crawler.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

#include <filesystem>
#include <unordered_set>

std::vector<std::string>
Crawler::extract_links_(const std::string &html, const std::string &base_url) {
	std::vector<std::string> links;
    std::unordered_set<std::string> seen;

    size_t pos = 0;
    while (pos < html.size()) {
        // Find "href" (case-insensitive check for 'h','H')
        pos = html.find("href", pos);
        if (pos == std::string::npos) break;
        pos += 4; // skip "href"

        // Skip optional whitespace and '='
        while (pos < html.size() && (html[pos] == ' ' || html[pos] == '\t'))
            pos++;
        if (pos >= html.size() || html[pos] != '=') continue;
        pos++; // skip '='
        while (pos < html.size() && (html[pos] == ' ' || html[pos] == '\t'))
            pos++;

        if (pos >= html.size()) break;
        char quote = html[pos];
        if (quote != '"' && quote != '\'') continue;
        pos++; // skip opening quote

        size_t end = html.find(quote, pos);
        if (end == std::string::npos) break;

        std::string raw_link(html, pos, end - pos);
        pos = end + 1;

        if (raw_link.empty() || raw_link.size() > 2048) continue;

        // Resolve relative URL
        std::string resolved = resolve_url(base_url, raw_link);
        if (!(resolved.starts_with("http://") || resolved.starts_with("https://")))
            continue;
        resolved = normalize_url(resolved);
        if (resolved.empty()) continue;

        if (!seen.contains(resolved)) {
            seen.insert(resolved);
            links.push_back(resolved);
        }
    }
    return links;
}


Crawler::Crawler(CrawlConfig config):
	config_(std::move(config)),
	frontier_(),
	robots_(),
	pool_(config_.num_threads)
{}

void Crawler::run() {
	std::filesystem::create_directories(config_.output_dir);
	// For now this pushes the initial seeded URLs to the frontier, spawns crawl worker thereads and exits
	std::cout << "[crawler] loading " << config_.seed_urls.size() << " seed URLs" << std::endl;
	for (const auto& url : config_.seed_urls) {
		std::cout << "[seed] " << url << std::endl;
	}
	frontier_.push_multiple(config_.seed_urls);
	for (size_t i = 0; i < config_.num_threads; i++) {
		pool_.submit([this]{
				crawl_worker_();
				});
	}
	// monitor CRAWL progress some way
	while (pages_crawled_.load() < config_.max_pages)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    frontier_.mark_done();
	pool_.shutdown();
}

void Crawler::crawl_worker_() {
	while (true) {
		if (pages_crawled_.load() >= config_.max_pages)
			break;
		auto url_opt = frontier_.pop();
		if (!url_opt.has_value())
			break;
		// robots.txt check
		std::string url = url_opt.value();
		bool allowed = robots_.is_allowed(url);
		if (!allowed)
			continue;
		// fetch and validate response
		auto fetched_page = fetch(url);
		if (fetched_page) {
			auto &resp = fetched_page.value();
			if (resp.status_code != 200)
				continue;
			if (resp.content_type.find("text/html") == std::string::npos)
				continue;

			auto page_num = pages_crawled_.fetch_add(1);
			if (page_num >= config_.max_pages) {
				pages_crawled_.fetch_sub(1);

				break;
			}
			auto doc_id = next_doc_id_.fetch_add(1);
			//auto file_name = generate_save_file_path_(url);
			std::optional<std::string> file_name = config_.output_dir + "/" + std::to_string(doc_id) + ".html";

			std::ofstream out(file_name.value());
			if (!out.is_open()) {
				std::cerr << "failed to open " << file_name.value() << std::endl;
				continue;
			}
			out << resp.body;
			if (!out.good()) {
				std::cerr << "failed to write " << file_name.value() << std::endl;
				continue;
			}
			// save html and extract links
			std::string html_copy = resp.body;
			std::vector<std::string> extracted_urls = extract_links_(html_copy, url);
			// push discovered links
			if (!extracted_urls.empty()) {
				frontier_.push_multiple(extracted_urls);
			}
			// increment pages_crawled_ counter
			std::cout << "[" << page_num + 1 << "/" << config_.max_pages << "]" << url << " (" << resp.body.size() /1024 << " KB)" << std::endl;
			std::cout << "[extract] " << extracted_urls.size() << " links from " << url << std::endl;
		} else {
			std::cout<<"Failed to fetch data from URL - "<<url<<" Reason - "<<to_string(fetched_page.error())<<std::endl;
		}
	}
	// Marking completion since we have reached limit of page extraction or extracted all pages
}
