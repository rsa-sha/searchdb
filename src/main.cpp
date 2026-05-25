#include "common/timer.hpp"
#include "crawler/crawler.hpp"

#include "pipeline/doc_store.hpp"
#include "pipeline/html_parser.hpp"
#include "pipeline/tokenizer.hpp"

#include "index/doc_lengths.hpp"
#include "index/inverted_index.hpp"

#include "search/bm25_scorer.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

// HARD Limit of 10MB for max HTML page size
constexpr size_t MAX_HTML_SIZE = 10 * 1024 * 1024;


// If user passes no args or -h or --help
static void print_usage() {
	std::cerr <<
        "Usage:\n"
        "  ./searchdb crawl   --seeds=file --max-pages=100 --threads=4 --output=data/raw\n"
        "  ./searchdb process --input=data/raw --output=data/processed\n"
        "  ./searchdb index --docs=data/processed/docs.bin --output=data/index/\n"
		"  ./searchdb search --index=data/index/--docs=data/processed/docs.bin --query=\"machine learning\" --top=10\n";
}

// Turns tokenized vector of strings to string representation
static std::string join_tokens(const std::vector<std::string>& tokens) {
    if (tokens.empty()) return "";

    std::string out;
    out.reserve(256); // optional optimization

    out += tokens[0];

    for (size_t i = 1; i < tokens.size(); ++i) {
        out += ' ';
        out += tokens[i];
    }

    return out;
}

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

static int run_crawl(int argc, char **argv) {
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
        std::cerr << "--seeds is required" << "\n";
        return 1;
    }

    config.seed_urls = read_seed_file(seeds_file);

    std::cout << "[crawl] seeds=" << config.seed_urls.size() << "\n";
    Crawler crawler(config);
    crawler.run();
	return 0;
}

static int run_process(int argc, char **argv) {
	ScopedTimer total_timer("total process pipeline");
	std::string input_dir;
	std::string output_dir;
	for(int i=2;i <argc; i++) {
		std::string arg = argv[i];
		if (arg.starts_with("--input="))
			input_dir = arg.substr(8);
		if (arg.starts_with("--output="))
			output_dir = arg.substr(9);
	}
	if (input_dir.empty() || output_dir.empty()) {
		std::cerr<<"--input and --output are required" << "\n";
		return 1;
	}

	// Tokenizer tokenizer;

	DocStoreWriter writer;
	uint32_t doc_id = 0;
	size_t total_text_len = 0;
	size_t processed = 0;
	size_t skipped = 0;
	auto start = std::chrono::steady_clock::now();
	// Iterate over all files [since we're only keeping HTML in there no need to check]
	for(const auto &entry : std::filesystem::directory_iterator(input_dir)) {
		if (!entry.is_regular_file())
			continue;
		try {
			std::ifstream in(entry.path());
			if (!in.is_open()) {
				skipped++;
				continue;
			}
			//std::string html((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			in.seekg(0, std::ios::beg);

			if (size > MAX_HTML_SIZE) {
				skipped++;
				continue;
			}
			std::string html;
			html.resize(size);
			in.read(html.data(), size);
			auto parsed = parse_html(html);
			if (!parsed.has_value()) {
				skipped++;
				continue;
			}
			auto data = parsed.value();
			// check if data is there and should be tokenized
			if (data.body_text.empty()) {
				skipped++;
				continue;
			}
			/**************************************************************
			 ********************* SKIPPING FOR NOW ***********************
			 **************************************************************
			// Pass the parsed data through tokenizer and stemmer [returns std::vector<std::string>]
			auto tokenized_text = tokenizer.tokenize(data.body_text);
			if (tokenized_text.empty()) {
				skipped++;
				continue;
				}
			***************************************************************
			**************************************************************/
			Document doc;
			doc.doc_id = doc_id++;
			doc.url = entry.path().filename().string();
			doc.title = data.title;
			doc.text = data.body_text;

			total_text_len += data.body_text.size();
			writer.add(doc);
			processed++;

			if (processed%100 == 0) {
				std::cout << "[process] " << processed << " docs processed" << "\n";
			}
		} catch (const std::exception &e) {
			std::cerr << "[warn] failed: " << entry.path() << " reason: " << e.what() << "\n";
			skipped++;
		}
	}
	auto end = std::chrono::steady_clock::now();
	double seconds = std::chrono::duration<double>(end - start).count();
	double docs_per_sec = processed / seconds;
	double avg_len = processed ? (double)total_text_len / processed : 0.0;
	std::cout
    << "[process] done processed=" << processed
    << " skipped=" << skipped
    << " avg_len=" << avg_len
    << " docs/sec=" << docs_per_sec
    << " time=" << seconds << "s\n";


	// IMPORTANT: flush output
    std::string out_file = output_dir + "/docs.bin";
    auto result = writer.flush(out_file);

    if (!result.has_value()) {
        std::cerr << "[process] failed to write doc store" << "\n";
        return 1;
    }

    std::cout << "[process] done processed=" << processed << " skipped=" << skipped << "\n";

	// Verifying readability of docs
	DocStoreReader reader(out_file);

	std::cout << "\n[verify] reading 5 random docs:\n";

	for (int i = 0; i < 5; ++i) {
		uint32_t id = rand() % processed;

		auto doc = reader.get(id);
		if (!doc.has_value()) continue;

		const auto &d = doc.value();

		std::cout << "\nID: " << d.doc_id << "\n"
				  << "Title: " << d.title << "\n"
				  << "URL: " << d.url << "\n"
				  << "Text: "
				  << d.text.substr(0, 100)
				  << "...\n";
	}
	return 0;
}


static int run_index(int argc, char **argv) {
	ScopedTimer total_timer("Total time required to Build Index from DocStore");
	std::string docs_input_file;
	std::string docs_output_dir;
	for(int i=2;i <argc; i++) {
		std::string arg = argv[i];
		if (arg.starts_with("--docs="))
			docs_input_file = arg.substr(7);
		if (arg.starts_with("--output="))
			docs_output_dir = arg.substr(9);
	}
	if (docs_input_file.empty() || docs_output_dir.empty()) {
		std::cerr<<"--docs and --output are required" << "\n";
		return 1;
	}

	DocStoreReader reader(docs_input_file);

	Tokenizer tokenizer;
	InvertedIndexBuilder builder;

	for (uint32_t doc_id = 0; doc_id < reader.doc_count(); doc_id++) {
		auto result = reader.get(doc_id);
		if (!result)
			continue;

		const auto &doc = result.value();
		std::string combined = doc.title + " " + doc.text;
		auto tokens = tokenizer.tokenize(combined);
		builder.add_document(doc_id, tokens);

		if ((doc_id + 1) % 100 == 0)
			std::cout << "[index] processed " << (doc_id + 1) << " docs\n";
	}
	builder.finalize();

	auto [longest_term, longest_size] = builder.longest_posting_list();
	std::cout << "\n[index] Indexed " << reader.doc_count() << " documents\n"
			  << "[index] Vocabulary: " << builder.vocabulary_size() << " terms\n"
			  << "[index] Total postings: " << builder.total_postings() << "\n"
			  << "[index] Average doc length: " << builder.avg_doc_length() << "\n"
			  << "[index] Longest posting list: '" << longest_term
			  << "' → " << longest_size << " docs\n";

	auto it = builder.index().find("algorithm");
	if (it != builder.index().end())
		std::cout << "[sanity] 'algorithm' appears in "<< it->second.size() << " documents\n";

	std::string output_file = docs_output_dir + "/doc_lengths.bin";
	DocLengthWriter writer;
	writer.write(output_file, builder.doc_lengths());
	serialize_index(builder, docs_output_dir + "/inverted_index.bin");
	serialize_index_v2(builder, docs_output_dir + "/inverted_index_v2.bin");
	std::cout << "[index] wrote v2 mmap index: " << docs_output_dir + "/inverted_index_v2.bin" << "\n";
	return 0;
}

int run_search(int argc, char **argv) {
	std::string docs_file;
	std::string index_dir;
	std::string query;
	size_t top_k = 10;
	for(int i=2; i<argc; i++) {
		std::string arg = argv[i];

		if (arg.starts_with("--docs="))
            docs_file = arg.substr(7);
        else if (arg.starts_with("--index="))
            index_dir = arg.substr(8);
        else if (arg.starts_with("--query="))
            query = arg.substr(8);
        else if (arg.starts_with("--top="))
            top_k = std::stoul(arg.substr(6));
	}
	if (docs_file.empty() || query.empty()) {
        std::cerr << "--docs, --index and --query are required\n";
        return 1;
    }

    // 1. Load docstore
    DocStoreReader reader(docs_file);

    // 2. Load mmap-based index (zero-copy, no hashmap construction)
    Tokenizer tokenizer;
	std::string index_file = index_dir + "/" + "inverted_index_v2.bin";
	std::string doc_lengths_file = index_dir + "/" + "doc_lengths.bin";
	InvertedIndex index(index_file);

	// 3. Load doc_lengths
    DocLengthReader dl_reader(doc_lengths_file);

    // 4. Build scorer
    BM25Scorer scorer(
        index.doc_count(),
        index.avg_doc_length(),
        &dl_reader
    );
	std::cerr << "DEBUG doc_count = " << index.doc_count() << "\n";
	std::cerr << "DEBUG total_tokens avgdl = " << index.avg_doc_length() << "\n";
    // 5. Tokenize query
    auto query_terms = tokenizer.tokenize(query);

    if (query_terms.empty()) {
        std::cerr << "empty query after tokenization\n";
        return 1;
    }

    // 6. Run BM25
    auto results = scorer.query(query_terms, index, top_k);

    // 7. Print results
    std::cout << std::fixed;
    std::cout.precision(10);

    std::cout << "\n[query] " << query << "\n";
    std::cout << "[results] " << results.size() << " docs\n\n";

    for (size_t i = 0; i < results.size(); i++) {

        const auto &r = results[i];
        auto doc_opt = reader.get(r.doc_id);

        if (!doc_opt) continue;

        const auto &doc = doc_opt.value();

        std::string snippet =
            doc.text.substr(0, 120);

        std::cout
            << (i + 1)
            << ". score=" << r.score
            << " doc_id=" << r.doc_id
            << "\n";

        std::cout
            << "   title: " << doc.title << "\n";

        std::cout
            << "   text: " << snippet << "...\n\n";
    }

    return 0;
}



int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
		return 1;
    }

    std::string command = argv[1];

	if (command == "-h" || command == "--help") {
		print_usage();
		return 0;
	} else if (command == "crawl") {
		return run_crawl(argc, argv);
	} else if (command == "process") {
		return run_process(argc, argv);
	} else if (command == "index") {
		return run_index(argc, argv);
	} else if (command == "search") {
		return run_search(argc, argv);
	} else {
        std::cerr << "unknown command: " << command << "\n";
        return 1;
    }

    return 0;
}
