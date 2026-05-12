#include "crawler/crawler.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static std::vector<std::string>
read_seed_file(const std::string& path) {
    std::vector<std::string> seeds;
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error(
            "failed to open seeds file: " + path
        );
    }

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty())
            seeds.push_back(line);
    }

    return seeds;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage:\n" << "  ./searchdb crawl " << "--seeds=file " << "--max-pages=100 " << "--threads=4 " << "--output=data/raw\n";
        return 1;
    }

    std::string command = argv[1];

    if (command != "crawl") {
        std::cerr << "unknown command: " << command << std::endl;
        return 1;
    }

    CrawlConfig config;
    std::string seeds_file;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg.starts_with("--seeds="))
            seeds_file = arg.substr(8);
        else if (arg.starts_with("--max-pages="))
            config.max_pages = std::stoul(arg.substr(12));
        else if (arg.starts_with("--threads="))
			config.num_threads = std::stoul(arg.substr(10));
        else if (arg.starts_with("--output="))
            config.output_dir = arg.substr(9);
    }
    if (seeds_file.empty()) {
        std::cerr << "--seeds is required" << std::endl;
        return 1;
    }

    config.seed_urls = read_seed_file(seeds_file);

    std::cout << "[main] loaded " << config.seed_urls.size() << " seeds" << std::endl;
    Crawler crawler(config);
    crawler.run();
    return 0;
}
