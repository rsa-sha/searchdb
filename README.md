# search

An end-to-end search engine built from scratch in C++. Crawls the web, processes documents, builds an inverted index, and serves ranked search results using BM25 scoring.

```
Seed URLs
   |
   v
Crawler (multi-threaded, robots.txt, politeness)
   |
   v
Raw HTML files (data/raw/)
   |
   v
Processing Pipeline (Gumbo HTML parser, Porter stemmer)
   |
   v
Binary DocStore (mmap'd, O(1) random access)
   |
   v
Inverted Index Builder (posting lists, term frequencies)
   |
   v
BM25 Scorer (ranked keyword search)
```

## Demo

[![Crawl](https://asciinema.org/a/Up3u50F195wTUYji.svg)](https://asciinema.org/a/Up3u50F195wTUYji)
[![Process](https://asciinema.org/a/Qs7UwZPl4wrEcDn9.svg)](https://asciinema.org/a/Qs7UwZPl4wrEcDn9)
[![Index](https://asciinema.org/a/UYvACZvqGspMfUCw.svg)](https://asciinema.org/a/UYvACZvqGspMfUCw)
[![Search](https://asciinema.org/a/oCmIgqoLUUuivVTI.svg)](https://asciinema.org/a/oCmIgqoLUUuivVTI)

## Quick Start

### Build

The easiest way is to use the provided build scripts:

```bash
./script_build.sh          # Release build (optimized) -> build_release/
./script_build_asan.sh     # AddressSanitizer build    -> build_asan/
./script_build_tsan.sh     # ThreadSanitizer build     -> build_tsan/
./script_build_debug.sh    # Debug build               -> build_debug/
```

`script_build.sh` creates `build_release/` with compiler optimizations enabled — use this for testing and benchmarking.

Or use CMake directly if you prefer custom flags:

```bash
cmake -B build_release -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build_release
```

### Run

```bash
# 1. Crawl
./build_release/searchdb crawl --seeds=tools/crawl_seeds.txt --max-pages=1000 --threads=8 --output=data/raw

# 2. Process
./build_release/searchdb process --input=data/raw --output=data/processed

# 3. Index
./build_release/searchdb index --docs=data/processed/docs.bin --output=data/index/

# 4. Search
./build_release/searchdb search --index=data/index/ --docs=data/processed/docs.bin --query="inverted index"
```

## What's Inside

### Crawler (`src/crawler/`)

Multi-threaded web crawler using libcurl.

- **ThreadPool** with bounded work queue and backpressure
- **URL frontier** with per-domain politeness delays and dedup
- **robots.txt** parser and enforcer (User-agent: *, Disallow, Crawl-delay)
- **URL normalization** (RFC 3986: lowercasing, default port removal, fragment stripping, percent-decoding)
- **URL resolution** (relative -> absolute: protocol-relative, root-relative, path-relative)
- BFS traversal with configurable depth limit, 8 concurrent worker threads

### Document Processing (`src/pipeline/`)

Transforms raw HTML into structured, searchable documents.

- **HTML parser** (Gumbo): DOM traversal, title/body/link extraction, script/style/noscript filtering, entity decoding
- **Tokenizer**: non-alphanumeric splitting, lowercasing, 150 English stop words
- **Porter stemmer**: plural rules (s/es/ies), past tense (ed), gerunds (ing), derivational suffixes (ful/ness/ment), doubled consonant collapsing
- **DocStore**: binary serialization format `[doc_count][offset_table][document_blobs]` with mmap'd reader for zero-copy O(1) random access

### Inverted Index (`src/index/`)

In-memory inverted index with corpus statistics for ranking.

- Posting lists: `term -> [(doc_id, term_freq), ...]`, sorted by doc_id
- Document length tracking for BM25 length normalization
- Vocabulary statistics: term count, total postings, avg document length

### Search (`src/search/`)

BM25 keyword search over the inverted index.

- **BM25 scoring**: IDF weighting, TF saturation (k1=1.2), document length normalization (b=0.75)
- Top-K retrieval via min-heap
- Query tokenization using the same pipeline as indexing (consistency guarantee)
- Results display: doc_id, score, title, text snippet

## Performance

Measured on a single machine (Ubuntu, 8 cores):

| Stage | Metric | Value |
|-------|--------|-------|
| Crawl | Throughput | ~5 pages/sec (network-bound) |
| Crawl | Scale | 10K pages, 8 threads |
| Process | Throughput | 106 docs/sec (single-threaded) |
| Index | Build time | 5.6s for 9,939 documents |
| Index | Vocabulary | 447,539 terms |
| Index | Postings | 8,278,090 |
| Index | Avg doc length | 2,313 tokens |

## Testing

28 tests covering all components. To run them, go into whichever build directory was created by the script or your custom CMake command:

```bash
cd build_release/
ctest            # summary output
ctest -V         # verbose — shows individual test output
```

| Component | Tests | What's covered |
|-----------|-------|---------------|
| ThreadPool | 3 | Basic execution, stress (4000 tasks), clean shutdown |
| HTTP client | 2 | Timeout handling, redirect following |
| URL utils | 4 | Normalization, domain extraction, resolution, edge cases |
| URL frontier | 4 | Basic queue, done signal, politeness delay, threaded (8 threads + TSAN) |
| robots.txt | 3 | Basic rules, crawl-delay, invalid/missing files |
| Crawler | 1 | End-to-end crawl (real network) |
| HTML parser | 2 | Unit tests, integration with real crawled pages |
| Tokenizer | 2 | Unit tests, integration with parsed HTML |
| Stemmer | 1 | Porter stemmer rules |
| DocStore | 2 | Round-trip correctness, edge cases (empty strings, large docs) |
| Inverted index | 3 | Unit test, HTML integration, DocStore integration |
| BM25 scorer | 1 | Ranking correctness |

### Memory Safety

All pipelines validated:

- **Valgrind**: zero leaks, zero errors (crawl + process pipelines)
- **AddressSanitizer**: heap safety, buffer overflows, use-after-free
- **ThreadSanitizer**: data race detection on concurrent crawl + URL frontier

## Project Structure

```
searchdb/
├── src/
│   ├── common/
│   │   ├── mmap_file.hpp        # Memory-mapped file abstraction
│   │   ├── result.hpp           # Result<T> error handling (std::expected)
│   │   ├── thread_pool.hpp/cpp  # Bounded ThreadPool with backpressure
│   │   └── timer.hpp            # Scoped profiling timer
│   ├── crawler/
│   │   ├── crawler.hpp/cpp      # Multi-threaded BFS web crawler
│   │   ├── http_client.hpp/cpp  # libcurl fetch wrapper
│   │   ├── robots.hpp/cpp       # robots.txt parser + cache
│   │   ├── url_frontier.hpp/cpp # Thread-safe URL queue with politeness
│   │   └── url_utils.hpp/cpp    # URL normalization + resolution
│   ├── index/
│   │   ├── inverted_index.hpp/cpp  # Posting list builder
│   │   └── doc_lengths.hpp/cpp     # Document length metadata
│   ├── pipeline/
│   │   ├── doc_store.hpp/cpp    # Binary document storage (writer + mmap reader)
│   │   ├── html_parser.hpp/cpp  # Gumbo-based HTML -> text extraction
│   │   └── tokenizer.hpp/cpp    # Tokenizer + Porter stemmer
│   ├── search/
│   │   └── bm25_scorer.hpp/cpp  # BM25 ranking + top-K retrieval
│   └── main.cpp                 # CLI: crawl / process / index / search
├── tests/                       # 28 test files
├── tools/
│   ├── crawl_seeds.txt          # 30 Wikipedia seed URLs
│   └── wiki_extract.py          # Wikipedia XML dump extractor
├── docs/
│   ├── architecture.md          # System architecture
│   ├── design-decisions.md      # Engineering tradeoffs
│   └── log.md                   # Daily development log
├── CMakeLists.txt
├── script_build.sh              # Release build
├── script_build_asan.sh         # AddressSanitizer build
├── script_build_debug.sh        # Debug build
└── script_build_tsan.sh         # ThreadSanitizer build
```

## Dependencies

- **libcurl**: HTTP fetching
- **Gumbo**: HTML parsing
- **CMake** >= 3.20
- **Ninja** (recommended) or Make
- C++23 compiler (GCC 13+ or Clang 17+)

## Design Decisions

See [docs/design-decisions.md](docs/design-decisions.md) for detailed rationale. Highlights:

- **Pre-sized bulk file reads** over iterator-based loading — 1.7x pipeline speedup (measured)
- **Manual HTML link extraction** instead of `std::regex` — regex caused stack overflow on pages >500KB
- **mmap for DocStore reads** — zero-copy, O(1) random access, OS-managed page caching
- **In-memory inverted index** — simple, fast for 10K-scale corpus; disk serialization planned
- **Porter stemmer subset** — 10+ rules covering 90%+ of English morphology without a full NLP library

## Roadmap

- [ ] Index serialization to disk (varint + delta encoding)
- [ ] Pipeline parallelization (target: >500 docs/sec)
- [ ] Vector search with embedding similarity (sentence-transformers + SIMD dot product)
- [ ] HTTP query server (cpp-httplib)
- [ ] Hybrid retrieval (BM25 + vector via Reciprocal Rank Fusion)
- [ ] Scale testing to 100K+ documents (Wikipedia dump)

## Development Log

The full day-by-day development log with readings, implementation notes, test results, and profiling data is in [docs/log.md](docs/log.md).

## License

MIT
