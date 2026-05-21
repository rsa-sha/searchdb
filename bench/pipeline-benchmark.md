# 100K Document Pipeline Benchmark
```
Date: 2026-05-21  
Dataset Size: ~100,000 crawled documents  
Processed Documents: 99,212  
Skipped Documents: 788  
```

# System Overview

Pipeline:
```text
crawl -> process -> docs.bin -> inverted index -> BM25 search
```
## Current architecture:
- Multi-threaded crawler
- HTML parsing via Gumbo
- Custom tokenizer + stemmer
- Binary mmap-backed document store
- In-memory inverted index
- BM25 ranking

## Crawl Statistics

### Configuration
- Threads: 8
- Dataset target: 100K pages
- Primarily Wikipedia + linked external pages

### Observations
- Crawl throughput scales linearly
- Robots.txt handling functional
- URL normalization and dedup working
- External domains discovered correctly

> Crawler remains network-bound

## Processing Pipeline Benchmark
Command:
```bash
./searchdb process --input=data/raw --output=data/processed
```
Output:
```bash
[process] done processed=99212 skipped=788 avg_len=29502.8 docs/sec=118.943 time=834.113s
```
Metrics
|Metric|	            Value|
|---|---|
|Processed Docs|        99,212|
|Skipped Docs|	        788|
|Avg Extracted Length|	29,502 chars|
|Throughput|	        118.9 docs/sec|
|Total Time|	        834s (~13.9 min)|


## Indexing Benchmark
Command:
```bash
./searchdb index --docs=data/processed/docs.bin --output=data/index/
```
Output:
```bash
[index] Indexed 99212 documents
[index] Vocabulary: 2509065 terms
[index] Total postings: 92082983
[index] Average doc length: 2942.14
[index] Longest posting list: 'http' → 86873 docs
[sanity] 'algorithm' appears in 12805 documents
```
Metrics
|Metric|	Value|
|---|---|
|Indexed Docs|      99,212|
|Vocabulary Size|   2,509,065|
|Total Postings|    92,082,983|
|Avg Doc Length|    2,942 tokens|
|Build Time|        ~82.7s|

## Search Benchmark
1. Query 1 - `./searchdb search --query="inverted index"`
    
    Latency
    `real    0m3.972s`
    
    Top results:
    - Inverted index Wikipedia pages
    - IR-related documentation
    - Patent references
    
    BM25 ranking appears functional.

2. Query 2 - `./searchdb search --query="this is somewhat longer message to check for multiple queries and needs info on relative ranking"`

    Latency
    `real    0m3.943s`

    Observations
    - Query tokenization works
    - Multi-term scoring works
    - High-frequency terms dominate scoring
    - RFC documents heavily ranked due to broad term overlap


## What is Working Well

### Pipeline Separation
Clean stage separation:
- Crawl
- Process
- Store
- Index
- Search

Allows:
- Offline experimentation
- Deterministic indexing
- Rebuild independence
---
Stability
- 100K pipeline completed successfully
- No crawler crashes
- No regex stack overflows
- Thread pool stable under load
- mmap docstore functioning correctly
---
Search Quality

Current BM25 ranking produces mostly relevant results despite:
- no phrase search
- no field weighting
- no PageRank
- no semantic ranking

## Known Issues / TODOs
1. Vocabulary Explosion
    
    Vocabulary size: `2,509,065 terms`
    
    This is far too large for 100K documents, large amount of index pollution from:
    - URLs
    - hashes
    - HTML artifacts
    - CSS class names
    - encoded identifiers
    - multilingual fragments
    - random numeric tokens

    Impact
    - Excessive RAM usage
    - Larger posting lists
    - Slower indexing
    - Slower queries
    - Poorer ranking quality

    Proposed Fixes
    - Drop tokens >32 chars
    - Skip symbol-heavy tokens
    - Skip excessive digit patterns
    - Better normalization rules
    - Optional df==1 pruning

2. No HTML Boilerplate Removal
    
    Extracted text still contains navigation/UI content: `Jump to content Main menu Main menu move to sidebar hide`

    Impact
    - Polluted token stream
    - Worse ranking
    - Larger vocabulary
    - Increased index size

    Proposed Fixes
    - DOM-based boilerplate removal
    - Skip navigation/sidebar/footer sections
    - Content extraction heuristics

3. Very Slow Search Latency
    
    Current - `~4 seconds/query on 99K docs.`

    Current BM25 implementation:
    - scans large posting lists
    - scores too many documents
    - sorts excessively
    - performs heavy hashmap operations

    Proposed Fixes
    - Ignore ultra-high-df terms
    - Add candidate pruning
    - Add WAND / MaxScore later
    - Compress postings
    - Cache query structures

4. No Parallel Processing Pipeline

    Processing stage currently single-threaded, current throughput - `~119 docs/sec`
    
    Proposed Fixes:
    - Parallel HTML parsing
    - Parallel tokenization
    - Batch writes to docstore