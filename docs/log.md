# This file holds the information on what I read, worked on a given day ##

### Day 1 [4th of May, 2026]

`Readings`
- Went through the plan to get an idea about what components will be part of this system and what things I've to learn from basics
- Read basics of [`Inverted Index`](https://grokipedia.com/page/Inverted_index) and how those work {not in detail}, with some basic examples on my end and the IR book reference
- [A first take at building an inverted index](https://nlp.stanford.edu/IR-book/html/htmledition/a-first-take-at-building-an-inverted-index-1.html) 

`Implementation Work`
- Setup basic dir strucutre and some template code for:
    error handling (result.hpp)
    performance observation (timer.hpp)
    memory as file abstraction (mmap.hpp)
- Read basics about `mmap`<br>
    [mmap(2) — Linux manual page](https://man7.org/linux/man-pages/man2/mmap.2.html)<br>
    [mmap(3p) — Linux manual page](https://man7.org/linux/man-pages/man3/mmap.3p.html)
- Read about Cmake and added basic `script_build.sh` to compile the binary, path will be `build/searchdb`
---
### Day 2 [5th of May, 2026]

`Readings`
- Started off with refresher readings on:
    - [std::mutex](https://en.cppreference.com/cpp/thread/mutex)
    - [std::condition_variable](https://cppreference.com/cpp/thread/condition_variable), and [spurions_wakeup](https://en.wikipedia.org/wiki/Spurious_wakeup)
    - [std::jthread](https://en.cppreference.com/cpp/thread/jthread)
- Read about [thread pool](https://grokipedia.com/page/Thread_pool) and producer-consumer pattern

`Implementation Work`
- ThreadPool class setup and basic design:
```
        external code
     (crawler / main / indexer)
               |
            submit()
               |
        +----------------+
        |   ThreadPool   |
        |                |
        | queue_         |
        |                |
        | workers_       |
        +----------------+
          |        |
     worker_loop  worker_loop
          |        |
      execute    execute
```
- Added tests for ThreadPool in `tests`
- CMakeLists.txt updated with new binaries for testing, can run tests using `ctest -V` in `build` dir:
```sh
sah@rsa-sha:~/code/exa/searchdb/build$  ctest -V
UpdateCTestConfiguration  from :/home/sah/code/exa/searchdb/build/DartConfiguration.tcl
UpdateCTestConfiguration  from :/home/sah/code/exa/searchdb/build/DartConfiguration.tcl
Test project /home/sah/code/exa/searchdb/build
Constructing a list of tests
Done constructing a list of tests
Updating test list for fixtures
Added 0 tests to meet fixture requirements
Checking test dependency graph...
Checking test dependency graph end
test 1
    Start 1: basic

1: Test command: /home/sah/code/exa/searchdb/build/threadpool_basic
1: Working Directory: /home/sah/code/exa/searchdb/build
1: Test timeout computed to be: 10000000
1: counter = 100
1/3 Test #1: basic ............................   Passed    0.00 sec
test 2
    Start 2: stress

2: Test command: /home/sah/code/exa/searchdb/build/threadpool_stress
2: Working Directory: /home/sah/code/exa/searchdb/build
2: Test timeout computed to be: 10000000
2: counter = 4000
2/3 Test #2: stress ...........................   Passed    0.02 sec
test 3
    Start 3: shutdown

3: Test command: /home/sah/code/exa/searchdb/build/threadpool_shutdown
3: Working Directory: /home/sah/code/exa/searchdb/build
3: Test timeout computed to be: 10000000
3: counter = 200
3/3 Test #3: shutdown .........................   Passed    0.06 sec

100% tests passed, 0 tests failed out of 3

Total Test time (real) =   0.08 sec
sah@rsa-sha:~/code/exa/searchdb/build$
```
---
### Day 3 [6th of May, 2026]

`Readings`
- Started with the [libcurl documentation](https://curl.se/libcurl/):
    - Read about [easy interface](https://curl.se/libcurl/c/), related methods:
        - [curl_easy_init](https://curl.se/libcurl/c/curl_easy_init.html)
        - [curl_easy_duphandle](https://curl.se/libcurl/c/curl_easy_duphandle.html)
        - [curl_easy_setopt](https://curl.se/libcurl/c/curl_easy_setopt.html) and the options:
            - [CURLOPT_URL](https://curl.se/libcurl/c/CURLOPT_URL.html)
            - [CURLOPT_WRITEFUNCTION](https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html)
            - [CURLOPT_TIMEOUT](https://curl.se/libcurl/c/CURLOPT_TIMEOUT.html)
            - [CURLOPT_FOLLOWLOCATION](https://curl.se/libcurl/c/CURLOPT_FOLLOWLOCATION.html)
            - [CURLOPT_MAXREDIRS](https://curl.se/libcurl/c/CURLOPT_MAXREDIRS.html)
            - [CURLOPT_USERAGENT](https://curl.se/libcurl/c/CURLOPT_USERAGENT.html)
        - [curl_easy_perform](https://curl.se/libcurl/c/curl_easy_perform.html)
- Read about the [write_callback](https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html#EXAMPLE) function

`Implementation Work`
- Implemented a `fetch()` wrapper over `curl easy interface` methods and `write_callback` for data writes to `HttpResponse`
- Added basic timeout, redirect and sanity test for the crawler
- CMakeLists.txt cleanup and test files addition work. Test results:
```bash
sah@rsa-sha:~/code/exa/searchdb$  ./script_build.sh
Generating build files
-- The C compiler identification is GNU 13.3.0
-- The CXX compiler identification is GNU 13.3.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/lib/ccache/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/lib/ccache/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Found CURL: /usr/lib/x86_64-linux-gnu/libcurl.so (found version "8.5.0")
-- Configuring done (0.3s)
-- Generating done (0.0s)
-- Build files have been written to: /home/sah/code/exa/searchdb/build
Building
ninja: Entering directory `build'
[16/16] Linking CXX executable crawler_http_redirect_test
Build successful
```
```bash
sah@rsa-sha:~/code/exa/searchdb$  cd build/
sah@rsa-sha:~/code/exa/searchdb/build$  ./searchdb
searchdb v0.0.1
hello.mmap
Fetching data from url -> https://en.wikipedia.org/wiki/Web_crawler
200
256836
Fetched data successfully
sah@rsa-sha:~/code/exa/searchdb/build$  ctest
Test project /home/sah/code/exa/searchdb/build
    Start 1: threadpool_basic
1/5 Test #1: threadpool_basic .................   Passed    0.00 sec
    Start 2: threadpool_stress
2/5 Test #2: threadpool_stress ................   Passed    0.01 sec
    Start 3: threadpool_shutdown
3/5 Test #3: threadpool_shutdown ..............   Passed    0.06 sec
    Start 4: crawler_http_timeout_test
4/5 Test #4: crawler_http_timeout_test ........   Passed   10.01 sec
    Start 5: crawler_http_redirect_test
5/5 Test #5: crawler_http_redirect_test .......   Passed    0.68 sec

100% tests passed, 0 tests failed out of 5

Total Test time (real) =  10.77 sec
sah@rsa-sha:~/code/exa/searchdb/build$ 
```
---
### Day 4 [7th of May, 2026]

`Readings`
- Read about:
    - [URI Normalization](https://grokipedia.com/page/URI_normalization):
    - [RFC 3986](https://datatracker.ietf.org/doc/html/rfc3986#section-4.1)
    - [URL syntax](https://developer.mozilla.org/en-US/docs/Web/API/URL)
    - [Why are URL case sensitive?](https://webmasters.stackexchange.com/questions/90339/why-are-urls-case-sensitive)
    - [Canonical URLs](https://developers.google.com/search/docs/crawling-indexing/consolidate-duplicate-urls)

`Implementation Work`
- Implemented `url_component_extraction()` based on the URL syntax guide
- Implemented `url normalization`, `url resolution` and `url domain extraction` methods
- CMakeLists.txt updated with new tests for URL related methods. Test results:
```bash

sah@rsa-sha:~/code/exa/searchdb/build$  ctest
Test project /home/sah/code/exa/searchdb/build
    Start 1: threadpool_basic
1/9 Test #1: threadpool_basic .................   Passed    0.00 sec
    Start 2: threadpool_stress
2/9 Test #2: threadpool_stress ................   Passed    0.01 sec
    Start 3: threadpool_shutdown
3/9 Test #3: threadpool_shutdown ..............   Passed    0.06 sec
    Start 4: crawler_http_timeout_test
4/9 Test #4: crawler_http_timeout_test ........   Passed   10.02 sec
    Start 5: crawler_http_redirect_test
5/9 Test #5: crawler_http_redirect_test .......   Passed    0.59 sec
    Start 6: url_normalize_test
6/9 Test #6: url_normalize_test ...............   Passed    0.00 sec
    Start 7: url_domain_test
7/9 Test #7: url_domain_test ..................   Passed    0.00 sec
    Start 8: url_resolve_test
8/9 Test #8: url_resolve_test .................   Passed    0.00 sec
    Start 9: url_utils_test
9/9 Test #9: url_utils_test ...................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 9

Total Test time (real) =  10.69 sec
sah@rsa-sha:~/code/exa/searchdb/build$
```
---
### Day 5 [8th of May, 2026]

`Readings`
- Read about [URL/Crawl Frontier](https://grokipedia.com/page/crawl_frontier):
- IR-Book:[The URL Frontier](https://nlp.stanford.edu/IR-book/html/htmledition/the-url-frontier-1.html)
- IR-Book:[Crawler Architecture](https://nlp.stanford.edu/IR-book/html/htmledition/crawler-architecture-1.html)
- Paper titled [Design and Implementation of a High-Performance Distributed Web Crawler](https://resources.mpi-inf.mpg.de/d5/teaching/ss05/is05/papers/webcrawler.pdf)

`Implementation Work`
- Implemented `URLFrontier` class with basic url(s) additon and fectching[`pop()`] methods and politeness timers
- CMakeLists.txt updated with new tests for URLFrontier related methods. Test results:
```bash

sah@rsa-sha:~/code/exa/searchdb/build$  ctest -R "^url_frontier"
Test project /home/sah/code/exa/searchdb/build
    Start 10: url_frontier_basic
1/4 Test #10: url_frontier_basic ...............   Passed    0.00 sec
    Start 11: url_frontier_done
2/4 Test #11: url_frontier_done ................   Passed    0.00 sec
    Start 12: url_frontier_delay
3/4 Test #12: url_frontier_delay ...............   Passed    0.21 sec
    Start 13: url_frontier_threaded
4/4 Test #13: url_frontier_threaded ............   Passed   20.48 sec

100% tests passed, 0 tests failed out of 4

Total Test time (real) =  20.69 sec
sah@rsa-sha:~/code/exa/searchdb/build$
```
---
### Day 6 [9th of May, 2026]

`Readings`
- Re-read/reviewed parts of the code that's written till now
- Read about [robots.txt](https://grokipedia.com/page/Robots.txt), it's structure, syntax and parameters
- Went through robots.txt of [github](https://github.com/robots.txt) and [google](https://www.google.com/robots.txt) to get ideas for implementation work
- Read about [Sitemaps](https://grokipedia.com/page/Site_map) to know what're they for, not implementing for now.

`Implementation Work`
- Implemented `RobotsChecker` class with basic methods to check crawl allowance, crawl_delay timer, parsing `robots.txt` and in memory cache of the same called `robots_cache_`
- CMakeLists.txt updated with basic tests for RobotsChecker & related methods. Test results:
```bash
sah@rsa-sha:~/code/exa/searchdb/build$  ctest -R "^robots"
Test project /home/sah/code/exa/searchdb/build
    Start 14: robots_basic_test
1/3 Test #14: robots_basic_test ................   Passed    0.42 sec
    Start 15: robots_delay_test
2/3 Test #15: robots_delay_test ................   Passed    0.43 sec
    Start 16: robots_invalid_test
3/3 Test #16: robots_invalid_test ..............   Passed    0.02 sec

100% tests passed, 0 tests failed out of 3

Total Test time (real) =   0.87 sec
sah@rsa-sha:~/code/exa/searchdb/build$
```
---

### Day 7 [10th of May, 2026]

#### Readings
- Reviewed the crawler implementation and investigated issues related to multithreaded crawling and file persistence.

#### Implementation Work
- Updated the `searchdb` binary to support crawling from a pre-seeded URL list (`tools/crawl_seeds.txt`).
- Added support for:
  - configurable crawl limits (`--max-pages`)
  - configurable worker threads (`--threads`)
  - configurable output directory (`--output`)
- Successfully crawled and persisted 50 HTML pages concurrently using 4 crawler threads.
- Verified:
  - robots.txt fetching and enforcement
  - URL frontier expansion from extracted links
  - HTML persistence into `data/raw`
  - multithreaded crawl execution
- Fixed a crawl accounting bug where pages were counted before validating successful HTML persistence, which caused missing output files during concurrent crawling.

- Added basic crawler test, results:
```bash
sah@rsa-sha:~/code/exa/searchdb/build$  ctest -R "crawl_test$"
Test project /home/sah/code/exa/searchdb/build
    Start 17: crawler_run_basic_crawl_test
1/1 Test #17: crawler_run_basic_crawl_test .....   Passed    4.40 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   4.40 sec
sah@rsa-sha:~/code/exa/searchdb/build$
```
<details>
<summary>Crawler run output</summary>

```bash
sah@rsa-sha:~/code/exa/searchdb$  ./build/searchdb crawl --seeds=tools/crawl_seeds.txt --max-pages=50 --threads=4 --output=data/raw
[main] loaded 30 seeds
[crawler] loading 30 seed URLs
[seed] https://en.wikipedia.org/wiki/Web_crawler
[seed] https://en.wikipedia.org/wiki/Search_engine
[seed] https://en.wikipedia.org/wiki/Inverted_index
[seed] https://en.wikipedia.org/wiki/PageRank
[seed] https://en.wikipedia.org/wiki/Distributed_system
[seed] https://en.wikipedia.org/wiki/Database
[seed] https://en.wikipedia.org/wiki/B-tree
[seed] https://en.wikipedia.org/wiki/Trie
[seed] https://en.wikipedia.org/wiki/Latency
[seed] https://en.wikipedia.org/wiki/HTTP
[seed] https://en.wikipedia.org/wiki/Kernel_(operating_system)
[seed] https://en.wikipedia.org/wiki/Linux
[seed] https://en.wikipedia.org/wiki/Computer_cluster
[seed] https://en.wikipedia.org/wiki/Load_balancing_(computing)
[seed] https://en.wikipedia.org/wiki/MapReduce
[seed] https://en.wikipedia.org/wiki/Concurrency_(computer_science)
[seed] https://en.wikipedia.org/wiki/Multithreading_(computer_architecture)
[seed] https://en.wikipedia.org/wiki/Memory_management
[seed] https://en.wikipedia.org/wiki/Virtual_memory
[seed] https://en.wikipedia.org/wiki/File_system
[seed] https://en.wikipedia.org/wiki/RAID
[seed] https://en.wikipedia.org/wiki/TCP
[seed] https://en.wikipedia.org/wiki/IP_address
[seed] https://en.wikipedia.org/wiki/DNS
[seed] https://en.wikipedia.org/wiki/Compiler
[seed] https://en.wikipedia.org/wiki/C%2B%2B
[seed] https://en.wikipedia.org/wiki/Redis
[seed] https://en.wikipedia.org/wiki/ClickHouse
[seed] https://en.wikipedia.org/wiki/Elasticsearch
[seed] https://en.wikipedia.org/wiki/Vector_database
[robots] fetching robots.txt for en.wikipedia.org
[1/50]https://en.wikipedia.org/wiki/Inverted_index (79 KB)
[extract] 142 links from https://en.wikipedia.org/wiki/Inverted_index
[2/50]https://en.wikipedia.org/wiki/Web_crawler (250 KB)
[extract] 509 links from https://en.wikipedia.org/wiki/Web_crawler
[4/50]https://en.wikipedia.org/wiki/Search_engine (313 KB)
[extract] 658 links from https://en.wikipedia.org/wiki/Search_engine
[3/50]https://en.wikipedia.org/wiki/PageRank (418 KB)
[extract] 578 links from https://en.wikipedia.org/wiki/PageRank
[robots] fetching robots.txt for creativecommons.org
[5/50]https://en.wikipedia.org/wiki/Distributed_system (308 KB)
[extract] 722 links from https://en.wikipedia.org/wiki/Distributed_system
[6/50]https://en.wikipedia.org/wiki/Database (374 KB)
[extract] 1130 links from https://en.wikipedia.org/wiki/Database
[7/50]https://en.wikipedia.org/wiki/B-tree (263 KB)
[extract] 435 links from https://en.wikipedia.org/wiki/B-tree
[robots] fetching robots.txt for meta.wikimedia.org
[8/50]https://creativecommons.org/licenses/by-sa/4.0/deed.en (34 KB)
[extract] 111 links from https://creativecommons.org/licenses/by-sa/4.0/deed.en
[9/50]https://en.wikipedia.org/wiki/Trie (219 KB)
[extract] 496 links from https://en.wikipedia.org/wiki/Trie
[10/50]https://en.wikipedia.org/wiki/Latency (55 KB)
[extract] 93 links from https://en.wikipedia.org/wiki/Latency
[robots] fetching robots.txt for donate.wikimedia.org
[11/50]https://en.wikipedia.org/wiki/HTTP (408 KB)
[extract] 993 links from https://en.wikipedia.org/wiki/HTTP
[12/50]https://en.wikipedia.org/wiki/Kernel_(operating_system) (325 KB)
[extract] 655 links from https://en.wikipedia.org/wiki/Kernel_(operating_system)
[robots] fetching robots.txt for ar.wikipedia.org
[13/50]https://meta.wikimedia.org/ (141 KB)
[extract] 385 links from https://meta.wikimedia.org/
[14/50]https://donate.wikimedia.org/?wmf_source=donate&amp;wmf_medium=sidebar&amp;wmf_campaign=en.wikipedia.org&amp;uselang=en (56 KB)
[extract] 32 links from https://donate.wikimedia.org/?wmf_source=donate&amp;wmf_medium=sidebar&amp;wmf_campaign=en.wikipedia.org&amp;uselang=en
[16/50]https://ar.wikipedia.org/wiki/%D9%81%D9%87%D8%B1%D8%B3_%D9%85%D9%82%D9%84%D9%88%D8%A8 (100 KB)
[extract] 137 links from https://ar.wikipedia.org/wiki/%D9%81%D9%87%D8%B1%D8%B3_%D9%85%D9%82%D9%84%D9%88%D8%A8
[15/50]https://en.wikipedia.org/wiki/Linux (626 KB)
[extract] 1771 links from https://en.wikipedia.org/wiki/Linux
[17/50]https://en.wikipedia.org/wiki/Computer_cluster (203 KB)
[extract] 465 links from https://en.wikipedia.org/wiki/Computer_cluster
[robots] fetching robots.txt for az.wikipedia.org
[18/50]https://en.wikipedia.org/wiki/Load_balancing_(computing) (203 KB)
[extract] 352 links from https://en.wikipedia.org/wiki/Load_balancing_(computing)
[19/50]https://en.wikipedia.org/wiki/Concurrency_(computer_science) (130 KB)
[extract] 264 links from https://en.wikipedia.org/wiki/Concurrency_(computer_science)
[robots] fetching robots.txt for ca.wikipedia.org
[20/50]https://en.wikipedia.org/wiki/MapReduce (307 KB)
[extract] 1258 links from https://en.wikipedia.org/wiki/MapReduce
[21/50]https://az.wikipedia.org/wiki/T%C9%99rsin%C9%99_%C3%A7evrilmi%C5%9F_fayl (63 KB)
[extract] 93 links from https://az.wikipedia.org/wiki/T%C9%99rsin%C9%99_%C3%A7evrilmi%C5%9F_fayl
[22/50]https://en.wikipedia.org/wiki/Multithreading_(computer_architecture) (180 KB)
[extract] 696 links from https://en.wikipedia.org/wiki/Multithreading_(computer_architecture)
[23/50]https://ca.wikipedia.org/wiki/%C3%8Dndex_invertit (86 KB)
[extract] 120 links from https://ca.wikipedia.org/wiki/%C3%8Dndex_invertit
[24/50]https://en.wikipedia.org/wiki/Memory_management (173 KB)
[extract] 332 links from https://en.wikipedia.org/wiki/Memory_management
[25/50]https://en.wikipedia.org/wiki/Virtual_memory (216 KB)
[extract] 435 links from https://en.wikipedia.org/wiki/Virtual_memory
[robots] fetching robots.txt for cs.wikipedia.org
[27/50]https://en.wikipedia.org/wiki/TCP (64 KB)
[extract] 117 links from https://en.wikipedia.org/wiki/TCP
[26/50]https://en.wikipedia.org/wiki/File_system (380 KB)
[extract] 940 links from https://en.wikipedia.org/wiki/File_system
[robots] fetching robots.txt for de.wikipedia.org
[28/50]https://en.wikipedia.org/wiki/RAID (283 KB)
[extract] 506 links from https://en.wikipedia.org/wiki/RAID
[29/50]https://cs.wikipedia.org/wiki/Invertovan%C3%BD_soubor (59 KB)
[extract] 91 links from https://cs.wikipedia.org/wiki/Invertovan%C3%BD_soubor
[30/50]https://en.wikipedia.org/wiki/IP_address (231 KB)
[extract] 470 links from https://en.wikipedia.org/wiki/IP_address
[31/50]https://de.wikipedia.org/wiki/Invertierte_Datei (57 KB)
[extract] 86 links from https://de.wikipedia.org/wiki/Invertierte_Datei
[32/50]https://en.wikipedia.org/wiki/DNS (339 KB)
[extract] 671 links from https://en.wikipedia.org/wiki/DNS
[33/50]https://en.wikipedia.org/wiki/Compiler (312 KB)
[extract] 808 links from https://en.wikipedia.org/wiki/Compiler
[robots] fetching robots.txt for es.wikipedia.org
[34/50]https://en.wikipedia.org/wiki/Redis (187 KB)
[extract] 299 links from https://en.wikipedia.org/wiki/Redis
[35/50]https://en.wikipedia.org/wiki/C%2B%2B (410 KB)
[extract] 1400 links from https://en.wikipedia.org/wiki/C%2B%2B
[36/50]https://en.wikipedia.org/wiki/ClickHouse (104 KB)
[extract] 159 links from https://en.wikipedia.org/wiki/ClickHouse
[robots] fetching robots.txt for fi.wikipedia.org
[37/50]https://es.wikipedia.org/wiki/%C3%8Dndice_invertido (71 KB)
[extract] 101 links from https://es.wikipedia.org/wiki/%C3%8Dndice_invertido
[robots] fetching robots.txt for fr.wikipedia.org
[38/50]https://en.wikipedia.org/wiki/Elasticsearch (129 KB)
[extract] 207 links from https://en.wikipedia.org/wiki/Elasticsearch
[39/50]https://en.wikipedia.org/wiki/Vector_database (199 KB)
[extract] 385 links from https://en.wikipedia.org/wiki/Vector_database
[40/50]https://fi.wikipedia.org/wiki/K%C3%A4%C3%A4nteistiedosto (85 KB)
[extract] 97 links from https://fi.wikipedia.org/wiki/K%C3%A4%C3%A4nteistiedosto
[robots] fetching robots.txt for ja.wikipedia.org
[41/50]https://fr.wikipedia.org/wiki/Index_invers%C3%A9 (64 KB)
[extract] 99 links from https://fr.wikipedia.org/wiki/Index_invers%C3%A9
[robots] fetching robots.txt for ko.wikipedia.org
[robots] fetching robots.txt for no.wikipedia.org
[42/50]https://ja.wikipedia.org/wiki/%E8%BB%A2%E7%BD%AE%E3%82%A4%E3%83%B3%E3%83%87%E3%83%83%E3%82%AF%E3%82%B9 (87 KB)
[extract] 115 links from https://ja.wikipedia.org/wiki/%E8%BB%A2%E7%BD%AE%E3%82%A4%E3%83%B3%E3%83%87%E3%83%83%E3%82%AF%E3%82%B9
[43/50]https://ko.wikipedia.org/wiki/%EC%97%AD%EC%83%89%EC%9D%B8 (88 KB)
[extract] 127 links from https://ko.wikipedia.org/wiki/%EC%97%AD%EC%83%89%EC%9D%B8
[44/50]https://no.wikipedia.org/wiki/Invertert_indeks (70 KB)
[extract] 99 links from https://no.wikipedia.org/wiki/Invertert_indeks
[robots] fetching robots.txt for pt.wikipedia.org
[45/50]https://en.wikipedia.org/wiki/Main_Page (223 KB)
[extract] 641 links from https://en.wikipedia.org/wiki/Main_Page
[46/50]https://en.wikipedia.org/wiki (223 KB)
[extract] 641 links from https://en.wikipedia.org/wiki
[robots] fetching robots.txt for ru.wikipedia.org
[47/50]https://en.wikipedia.org/wiki/Wikipedia:Contents (138 KB)
[extract] 208 links from https://en.wikipedia.org/wiki/Wikipedia:Contents
[48/50]https://pt.wikipedia.org/wiki/Listas_invertidas (61 KB)
[extract] 94 links from https://pt.wikipedia.org/wiki/Listas_invertidas
[49/50]https://en.wikipedia.org/wiki/Portal:Current_events (339 KB)
[extract] 1679 links from https://en.wikipedia.org/wiki/Portal:Current_events
[50/50]https://ru.wikipedia.org/wiki/%D0%98%D0%BD%D0%B2%D0%B5%D1%80%D1%82%D0%B8%D1%80%D0%BE%D0%B2%D0%B0%D0%BD%D0%BD%D1%8B%D0%B9_%D0%B8%D0%BD%D0%B4%D0%B5%D0%BA%D1%81 (103 KB)
[extract] 112 links from https://ru.wikipedia.org/wiki/%D0%98%D0%BD%D0%B2%D0%B5%D1%80%D1%82%D0%B8%D1%80%D0%BE%D0%B2%D0%B0%D0%BD%D0%BD%D1%8B%D0%B9_%D0%B8%D0%BD%D0%B4%D0%B5%D0%BA%D1%81
sah@rsa-sha:~/code/exa/searchdb$  ls -l data/raw/ | grep html | wc -l
50
sah@rsa-sha:~/code/exa/searchdb$
```
</details>



### GDB Run args
```bash
gdb --args ./build/searchdb crawl --seeds=tools/crawl_seeds.txt --max-pages=1000 --threads=4 --output=data/raw
```
- Fixed the problem in `crawler.cpp` which used regex to extract URLs causing stack overflow in case of larger HTML bodies, replaced with manual string parsing, result of crawling 1000 pages
```bash
sah@rsa-sha:~/code/exa/searchdb$ ls -l data/raw | grep html| wc -l
1000
```
---
### Day 8 [11th of May, 2026]

`Readings`
- Read about HTML parsing, and the [BeautifulSoup](https://grokipedia.com/page/Beautiful_Soup_(HTML_parser)) parser

`Implementation Work`
- Implemented `parse_html` function, to run on crawled HTML data.
- Added tests [basic tests, previewing text from crawled html files] for html parsing. Test results:
```bash
sah@rsa-sha:~/code/exa/searchdb/build$  ctest -R "^html"
Test project /home/sah/code/exa/searchdb/build
    Start 18: html_parser_test
1/2 Test #18: html_parser_test .................   Passed    0.00 sec
    Start 19: html_parser_integration_test
2/2 Test #19: html_parser_integration_test .....   Passed    0.11 sec

100% tests passed, 0 tests failed out of 2

Total Test time (real) =   0.11 sec
sah@rsa-sha:~/code/exa/searchdb/build$
```
---
### Day 9 [12th of May, 2026]
`Readings`
- Read about tokenizing from [Information Retrieval](https://grokipedia.com/page/Information_retrieval#retrieval-process-and-components) standpoint
- Read the [tokenization](https://nlp.stanford.edu/IR-book/html/htmledition/tokenization-1.html), normalization & stemming from the [IR-Book](https://nlp.stanford.edu/IR-book/html/htmledition/the-term-vocabulary-and-postings-lists-1.html)
- Table of [Stopwords lists for technical language processing applications](https://pmc.ncbi.nlm.nih.gov/articles/PMC8341615/table/pone.0254937.t001/)

`Implementation Work`
- Implemented the `Tokenizer` class with basic tokenization methods
- Stop words have been picked up from a basic list that popped up during web search adn some from the stopwords list
- Added tests [basic tokenization test & previewing tokens from data in crawled html files] for html parsing. Test results:
```bash
sah@rsa-sha:~/code/exa/searchdb/build$  ctest -R "^token"
Test project /home/sah/code/exa/searchdb/build
    Start 20: tokenizer_test
1/2 Test #20: tokenizer_test ...................   Passed    0.00 sec
    Start 21: tokenizer_integration_test
2/2 Test #21: tokenizer_integration_test .......   Passed    0.13 sec

100% tests passed, 0 tests failed out of 2

Total Test time (real) =   0.13 sec
```

<details>
<summary>tokenizer_integration_test output</summary>

```bash
sah@rsa-sha:~/code/exa/searchdb$  ./build/tokenizer_integration_test

=================================
FILE: data/raw/0.html
TITLE: Inverted index - Wikipedia
TOKENS (first 100):
jump content main menu main menu move sidebar hide navigation main page contents current events random article wikipedia contact us contribute help learn edit community portal recent changes upload file special pages search search appearance donate create account log personal tools donate create account log contents move sidebar hide top applications compression see references external links toggle table contents inverted index 16 languages az rbaycanca catal tina deutsch espa ol suomi fran ais norsk bokm portugu rk edit links article talk english read edit view history tools tools move sidebar hide actions read edit view history general links related changes
TOKEN COUNT: 843

=================================
FILE: data/raw/1.html
TITLE: Search engine - Wikipedia
TOKENS (first 100):
jump content main menu main menu move sidebar hide navigation main page contents current events random article wikipedia contact us contribute help learn edit community portal recent changes upload file special pages search search appearance donate create account log personal tools donate create account log contents move sidebar hide top history toggle history subsection pre 1990s 1990s birth search engines 2000s present post dot com bubble approach toggle approach subsection local search market share toggle market share subsection russia east asia search engine bias customized results filter bubbles religious search engines search engine submission comparison social bookmarking technology toggle technology
TOKEN COUNT: 6236

=================================
FILE: data/raw/2.html
TITLE: Web crawler - Wikipedia
TOKENS (first 100):
jump content main menu main menu move sidebar hide navigation main page contents current events random article wikipedia contact us contribute help learn edit community portal recent changes upload file special pages search search appearance donate create account log personal tools donate create account log contents move sidebar hide top nomenclature overview crawling policy toggle crawling policy subsection selection policy restricting followed links url normalization path ascending crawling focused crawling academic focused crawler semantic focused crawler re visit policy politeness policy parallelization policy architectures security crawler identification crawling deep web visual vs programmatic crawlers list web crawlers toggle list web
TOKEN COUNT: 5433

=================================
FILE: data/raw/3.html
TITLE: PageRank - Wikipedia
TOKENS (first 100):
jump content main menu main menu move sidebar hide navigation main page contents current events random article wikipedia contact us contribute help learn edit community portal recent changes upload file special pages search search appearance donate create account log personal tools donate create account log contents move sidebar hide top description history algorithm toggle algorithm subsection simplified algorithm damping factor computation iterative power method implementation python variations toggle variations subsection pagerank undirected graph ranking objects two kinds distributed algorithm pagerank computation google toolbar serp rank google directory pagerank false spoofed pagerank manipulating pagerank directed surfer model uses toggle uses subsection
TOKEN COUNT: 6074

=================================
FILE: data/raw/4.html
TITLE: Distributed computing - Wikipedia
TOKENS (first 100):
jump content main menu main menu move sidebar hide navigation main page contents current events random article wikipedia contact us contribute help learn edit community portal recent changes upload file special pages search search appearance donate create account log personal tools donate create account log contents move sidebar hide top introduction patterns events vs messages parallel distributed computing history distributed computing architectures toggle distributed computing architectures subsection cell based architecture applications examples reactive distributed systems 10 theoretical foundations toggle theoretical foundations subsection 10 models 10 example 10 complexity measures 10 problems 10 election 10 properties distributed systems 10 topics 11
TOKEN COUNT: 5375

test_real_html_tokenization passed
ALL TOKENIZER TESTS PASSED
sah@rsa-sha:~/code/exa/searchdb$
```
</details>
---