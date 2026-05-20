# Architecture — searchdb

## Overview

`searchdb` is an end-to-end search engine pipeline that processes raw web pages into structured, storage-efficient documents.

The system currently focuses on ingestion + parsing + storage, with indexing and search planned as next phases.

---

## High-Level Data Flow
```
Seed URLs
↓
Crawler (HTTP + Frontier + Robots)
↓
Raw HTML files (data/raw/)
↓
Processing Pipeline
├── HTML Parser (Gumbo-based DOM traversal)
├── Text Cleaner + Entity Decoder
├── Tokenizer (present but currently bypassed)
↓
Structured Documents
↓
DocStoreWriter (binary serialization)
↓
DocStore file (data/processed/docs.bin)
↓
DocStoreReader (verification / random access)
```

---

## System Components


### 1. Crawler (`src/crawler/`)

Responsible for collecting raw web pages.

#### Modules

- `http_client.cpp`
  - Fetches HTML pages over HTTP
- `url_frontier.cpp`
  - BFS-style URL queue management
- `robots.cpp`
  - Enforces robots.txt rules
- `url_utils.cpp`
  - URL normalization and resolution
- `crawler.cpp`
  - Orchestrates crawling loop and thread management

#### Output

Raw HTML files stored as:
```
data/raw/
1.html
2.html
...
```

Each file corresponds to a fetched web page.

---

## 2. Processing Pipeline (`src/pipeline/`)

Transforms raw HTML into structured documents for storage.

---

### 2.1 HTML Parser (`html_parser.cpp`)

Uses **Gumbo HTML parser** to construct a DOM and extract structured fields.

### Extracted Fields

- Title (`<title>`)
- Body text (visible text nodes)
- Links (`<a href>`)

### Processing Behavior

- Recursive DOM traversal
- Skips non-content tags:
  - `<script>`
  - `<style>`
  - `<noscript>`
- Performs:
  - whitespace normalization
  - HTML entity decoding (`&amp;`, `&lt;`, etc.)
  - numeric entity decoding (`&#123;`)

### Output Structure

```cpp
struct ParsedDocument {
    std::string title;
    std::string body_text;
    std::vector<std::string> links;
};
```

### 2.2 Tokenizer (`tokenizer.cpp`)

Tokenizer exists but is currently not enabled in the main pipeline.

#### Intended responsibilities:
- token normalization
- splitting text into tokens
- optional stemming / stopword removal

### 2.3 DocStore Writer (doc_store.cpp)

Stores processed documents in a compact binary format.

File Layout
```
[u32 doc_count]
[u64 offsets[doc_count]]
[document blobs...]
```
Document Blob Format
```
[u16 url_len][url]
[u16 title_len][title]
[u32 text_len][text]
```
Design Goals
- Fast sequential writes
- O(1) random access via offset table
- Minimal parsing overhead during reads
### 2.4 DocStore Reader

Used for validation and debugging.

Features
- memory-mapped file access
- O(1) document lookup by ID
- zero-copy offset table reads

## 3. Common Utilities (src/common/)

ThreadPool (`thread_pool.cpp`)

A bounded task queue backed by worker threads (`std::jthread`).

Features
- Fixed-size worker pool
- Bounded queue with backpressure
- Condition-variable synchronization
- Safe shutdown handling

#### Intended Future Usage
Parallel execution of:
- HTML parsing
- tokenization
- indexing

#### `mmap_file.hpp`
Provides memory-mapped file abstraction for zero-copy reads.

#### `timer.hpp`

Scoped timing utility used for pipeline profiling.

Example output:
`[total process pipeline] elapsed: 93ms`

## 4. Inverted Index (src/index/)

The inverted index is the core structure used for full-text search and BM25 ranking.

It maps terms → documents containing the term, along with term frequency.

### 4.1 In-Memory Structure
```cpp
std::unordered_map<std::string, std::vector<Posting>>
```
Where:
```cpp
struct Posting {
    uint32_t doc_id;
    uint32_t term_freq;
};
```
### 4.2 Index Construction

Each document is processed as:
1. Tokenize title + body text
2. Count term frequencies per document
3. Add postings to index

Example:
```
doc_id = 5
"algorithm algorithm tree"
```
Becomes:
```
algorithm → (5, 2)
tree      → (5, 1)
```
### 4.3 Document Statistics
Tracked during indexing:
- doc_count → number of documents
- total_tokens → total tokens in corpus
- doc_lengths[doc_id] → number of tokens per document

These are required for BM25 scoring.

### 4.4 On-Disk Format (inverted_index.bin)
```
[doc_count: uint32]
[total_tokens: uint32]
[total_postings: uint64]

[num_terms: uint64]

For each term:
    [term_length: uint32]
    [term_bytes]

    [posting_count: uint32]

    For each posting:
        [doc_id: uint32]
        [term_freq: uint32]
```

## 5. Main Pipeline (`src/main.cpp`)
### Commands
#### Crawl
`./searchdb crawl --seeds=file --max-pages=10000 --threads=4 --output=data/raw`
#### Process
`./searchdb process --input=data/raw --output=data/processed`
#### Search
`./searchdb search --docs=data/processed/docs.bin --query="algorithm" --top=10`

---

#### Processing Flow
1. Iterate over raw HTML files
2. Load file into memory
3. Parse HTML → ParsedDocument
4. (Optional) tokenize text (currently disabled)
5. Write to DocStore
6. Log progress periodically

#### Performance Observations (via perf)
##### Hotspots
- `gumbo_parse`, `gumbo_lex` (HTML parsing overhead)
- UTF-8 validation routines
- malloc/free churn during DOM construction
- string concatenation in recursive DOM walk

##### Current Limitations
- Pipeline is single-threaded
- No batching or backpressure
- Tokenizer not integrated
- HTML entity decoding is minimal
- No indexing layer yet
- Full DOM is constructed in memory (no streaming parser)

## Planned Improvements
#### Phase 1 — Parallel Processing Pipeline
- Introduce ThreadPool into process stage
- Split pipeline into:
    - file loader (producer)
    - parser workers (consumers)
- Add queue-based backpressure

#### Phase 2 — Tokenization + Indexing
- integrate tokenizer into pipeline
- build inverted index (postings lists)
- optimize for query-time lookup

#### Phase 3 — Search Layer
- query parser
- ranking model (BM25 or custom scoring)
- top-k retrieval engine
