# Search Pipeline Overview

This document describes the end-to-end flow of documents through the
search engine.

---

# High-Level Flow

```text
Seed URLs
    ↓
URL Frontier
    ↓
Crawler
    ↓
robots.txt Validation
    ↓
HTTP Fetch
    ↓
HTML Parsing
    ↓
Tokenization
    ↓
Document Store
    ↓
Inverted Index
    ↓
BM25 Ranking
    ↓
Search Results
```

---

# Pipeline Stages

### 1. URL Frontier
Responsible for scheduling URLs to crawl<br>
Main responsibilities:
- deduplication
- politeness delays
- queue management
- retry handling

Implementation: 
- [src/crawler/url_frontier.cpp](../../src/crawler/url_frontier.cpp)

### 2. Crawling
Responsible for downloading pages<br>
Main responsibilities:
- HTTP GET requests
- redirects
- timeouts
- content validation

Implementation: 
- [src/crawler/crawler.cpp](../../src/crawler/crawler.cpp)
- [src/crawler/http_client.cpp](../../src/crawler/http_client.cpp)

### 3. robots.txt
Responsible for robots compliance<br>
Main responsibilities:
- fetching robots.txt
- caching robots rules
- validating crawl permissions

Implementation:
- [src/crawler/robots.cpp](../../src/crawler/robots.cpp)

### 4. HTML Parsing

Responsible for extracting text from HTML<br>
Main responsibilities:
- stripping tags
- decoding entities
- extracting metadata
- handling malformed HTML

Implementation:
- [src/pipeline/html_parser.cpp](../../src/pipeline/html_parser.cpp)


### 5. Tokenization
Responsible for converting text into searchable terms<br>
Main responsibilities:
- splitting text into tokens
- lowercasing
- punctuation removal
- normalization

Implementation:
- [src/pipeline/tokenizer.cpp](../../src/pipeline/tokenizer.cpp)

### 6. Document Storage

Responsible for storing raw documents and metadata<br>
Main responsibilities:
- document ID assignment
- metadata persistence
- content storage

Implementation:
- [src/pipeline/doc_store.cpp](../../src/pipeline/doc_store.cpp)


### 7. Indexing

Responsible for building the inverted index<br>
Main responsibilities:
- postings generation
- term frequency counting
- compression
- document length tracking

Implementation:
- [src/index/inverted_index.cpp](../../src/index/inverted_index.cpp)
- [src/index/doc_lengths.cpp](../../src/index/doc_lengths.cpp)


### 8. Ranking
Responsible for scoring search results<br>
Main responsibilities:
- BM25 scoring
- query term evaluation
- top-k retrieval

Implementation:
- [src/search/bm25_scorer.cpp](../../src/search/bm25_scorer.cpp)


---

### Threading Model

Current model:
- crawler workers run in parallel
- indexing is synchronous
- thread pool shared across crawler tasks

Implementation:
- [src/common/thread_pool.cpp](../../src/common/thread_pool.cpp)


---

### Storage Model
Current persistence:

memory-mapped files
compressed postings lists
sequential document IDs

Implementation:
- [src/common/mmap_file.hpp](../../src/common/mmap_file.hpp)


---
<!-- ### Future Improvements

Planned improvements:

- positional indexes
- phrase queries
- anchor text indexing
- query caching
- distributed indexing
 -->
