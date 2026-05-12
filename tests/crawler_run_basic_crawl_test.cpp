#include "../src/crawler/crawler.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main() {
    const std::string out_dir = "test_crawl_output";
    fs::remove_all(out_dir);
    fs::create_directories(out_dir);
    CrawlConfig config;
    config.seed_urls = {
        "https://en.wikipedia.org/wiki/Search_engine",
        "https://en.wikipedia.org/wiki/Web_crawler",
        "https://en.wikipedia.org/wiki/Inverted_index"
    };
    config.output_dir = out_dir;
    config.max_pages = 5;
    config.num_threads = 2;

    Crawler crawler(config);
    crawler.run();
    size_t html_count = 0;

    for (const auto& entry : fs::directory_iterator(out_dir)) {
        if (entry.path().extension() == ".html")
            html_count++;
    }
    assert(html_count > 0);
    fs::remove_all(out_dir);
    std::cout << "crawler_run_basic_crawl_test passed\n";
	return 0;
}
