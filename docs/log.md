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