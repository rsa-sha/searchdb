cmake_minimum_required(VERSION 3.20)

project(searchdb)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

# -------------------------
# Dependencies
# -------------------------
find_package(CURL REQUIRED)

enable_testing()

# -------------------------
# Libraries
# -------------------------
add_library(crawler
    src/crawler/http_client.cpp
)

add_library(url
    src/crawler/url_utils.cpp
)
target_link_libraries(crawler PRIVATE CURL::libcurl)

add_library(threadpool
    src/common/thread_pool.cpp
)

add_library(urlfrontier
    src/crawler/url_frontier.cpp
)
target_link_libraries(urlfrontier
	PUBLIC url
)

add_library(robots
	src/crawler/robots.cpp
)
target_link_libraries(robots
	PUBLIC
		url
		crawler
		CURL::libcurl
)

add_library(crawl_core
	src/crawler/crawler.cpp
)

target_link_libraries(crawl_core
	PUBLIC
		url
		crawler
		CURL::libcurl
		threadpool
		urlfrontier
		robots
		html_parser
)

add_library(html_parser
	src/pipeline/html_parser.cpp
)
target_link_libraries(html_parser
	PUBLIC gumbo
)

add_library(tokenizer
	src/pipeline/tokenizer.cpp
)

add_library(docstore
	src/pipeline/doc_store.cpp
)

# -------------------------
# Main executable
# -------------------------
add_executable(searchdb
    src/main.cpp
)

target_link_libraries(searchdb
    PRIVATE
        crawler
		crawl_core
        threadpool
        url
        CURL::libcurl
        gumbo
        docstore
)

# -------------------------
# Test helper macro
# -------------------------
function(add_searchdb_test name type)
    add_executable(${name}
        tests/${name}.cpp
    )

    if(type STREQUAL "thread")
        target_link_libraries(${name}
            PRIVATE threadpool
        )
    elseif(type STREQUAL "crawler")
        target_link_libraries(${name}
            PRIVATE crawler CURL::libcurl
        )
    elseif(type STREQUAL "url")
        target_link_libraries(${name}
            PRIVATE url
        )
    elseif(type STREQUAL "urlfrontier")
        target_link_libraries(${name}
			PRIVATE urlfrontier
        )
    elseif(type STREQUAL "robots")
        target_link_libraries(${name}
			PRIVATE robots
        )
    elseif(type STREQUAL "crawl_core")
        target_link_libraries(${name}
			PRIVATE crawl_core
        )
    elseif(type STREQUAL "html_parser")
        target_link_libraries(${name}
			PRIVATE html_parser
        )
    elseif(type STREQUAL "tokenizer")
        target_link_libraries(${name}
			PRIVATE tokenizer
        )
    elseif(type STREQUAL "docstore")
        target_link_libraries(${name}
			PRIVATE docstore
        )
    else()
        message(FATAL_ERROR "Unknown test type: ${type}")
    endif()

    add_test(NAME ${name} COMMAND ${name})
endfunction()

# -------------------------
# Tests
# -------------------------
# Threadpool tests
add_searchdb_test(threadpool_basic thread)
add_searchdb_test(threadpool_stress thread)
add_searchdb_test(threadpool_shutdown thread)

# Crawler tests
add_searchdb_test(crawler_http_timeout_test crawler)
add_searchdb_test(crawler_http_redirect_test crawler)

# URL tests
add_searchdb_test(url_normalize_test url)
add_searchdb_test(url_domain_test url)
add_searchdb_test(url_resolve_test url)
add_searchdb_test(url_utils_test url)

# URLFrontier tests
add_searchdb_test(url_frontier_basic urlfrontier)
add_searchdb_test(url_frontier_done urlfrontier)
add_searchdb_test(url_frontier_delay urlfrontier)
add_searchdb_test(url_frontier_threaded urlfrontier)
if(ENABLE_TSAN)
    target_compile_options(url_frontier_threaded PRIVATE
        -fsanitize=thread
        -fno-omit-frame-pointer
        -g
    )

    target_link_options(url_frontier_threaded PRIVATE
        -fsanitize=thread
    )
endif()

# Robots tests
add_searchdb_test(robots_basic_test robots)
add_searchdb_test(robots_delay_test robots)
add_searchdb_test(robots_invalid_test robots)

# Crawl core
add_searchdb_test(crawler_run_basic_crawl_test crawl_core)

# HTML Parser test
add_searchdb_test(html_parser_test html_parser)
add_searchdb_test(html_parser_integration_test html_parser)
set_tests_properties(html_parser_integration_test PROPERTIES
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Tokenizerer test
add_searchdb_test(tokenizer_test tokenizer)
add_searchdb_test(tokenizer_integration_test tokenizer)
target_link_libraries(tokenizer_integration_test PRIVATE html_parser)
set_tests_properties(tokenizer_integration_test PROPERTIES
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Stemmer test
add_searchdb_test(stemmer_test tokenizer)

# Doc store tests
add_searchdb_test(doc_store_test docstore)
add_searchdb_test(doc_store_edge_test docstore)
