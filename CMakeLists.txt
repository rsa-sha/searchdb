cmake_minimum_required(VERSION 3.20)

project(searchdb)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -------------------------
# Enable testing framework
# -------------------------
enable_testing()

# -------------------------
# Core executable
# -------------------------
add_executable(searchdb
    src/main.cpp
    src/common/thread_pool.cpp
)

# -------------------------
# ThreadPool tests
# -------------------------
add_executable(threadpool_basic
    tests/threadpool_basic.cpp
    src/common/thread_pool.cpp
)

add_executable(threadpool_stress
    tests/threadpool_stress.cpp
    src/common/thread_pool.cpp
)

add_executable(threadpool_shutdown
    tests/threadpool_shutdown.cpp
    src/common/thread_pool.cpp
)

# -------------------------
# Register tests with CTest
# -------------------------
add_test(NAME basic COMMAND threadpool_basic)
add_test(NAME stress COMMAND threadpool_stress)
add_test(NAME shutdown COMMAND threadpool_shutdown)
