cmake_minimum_required(VERSION 3.20)

project(searchdb)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# -------------------------
# Main executable
# -------------------------
add_executable(searchdb
    src/main.cpp
)

target_link_libraries(searchdb
    PRIVATE
        crawler
        threadpool
        url
        CURL::libcurl
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
