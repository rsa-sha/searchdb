## File Reading Strategy for Pipeline Ingestion

While processing large batches of HTML documents, iterator-based file loading:
```cpp
std::string html(
    std::istreambuf_iterator<char>(in),
    std::istreambuf_iterator<char>()
);
```
was replaced with a pre-sized bulk read:
```cpp
in.seekg(0, std::ios::end);
size_t size = in.tellg();
in.seekg(0, std::ios::beg);

std::string html;
html.resize(size);

in.read(html.data(), size);
```

Reason: The iterator-based approach reads data incrementally and may trigger repeated internal reallocations while the string grows.

The pre-sized approach:
- allocates memory once
- performs a single contiguous read
- avoids iterator overhead
- improves cache locality
- reduces allocator churn

Observed impact on ~1K HTML documents:
- iterator-based loading: ~14.4s
- pre-sized bulk read: ~8.1s
This reduced total pipeline runtime by roughly 1.7×.

<details>
<summary>DocStore Benchmark Output</summary>

```shell
sah@rsa-sha:~/code/exa/searchdb$  time ./build/searchdb process --input=data/raw/ --output=data/processed/
[process] 100 docs processed
[process] 200 docs processed
[process] 300 docs processed
[process] 400 docs processed
[process] 500 docs processed
[process] 600 docs processed
[process] 700 docs processed
[process] 800 docs processed
[process] 900 docs processed
[process] done processed=986 skipped=14

real    0m14.415s
user    0m14.328s
sys     0m0.089s
sah@rsa-sha:~/code/exa/searchdb$  ## with Pre-Size read
sah@rsa-sha:~/code/exa/searchdb$  ./script_build.sh
Generating build files
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /home/sah/code/exa/searchdb/build
Building
ninja: Entering directory `build`
[2/2] Linking CXX executable searchdb
Build successful
sah@rsa-sha:~/code/exa/searchdb$  rm data/processed/docs.bin
sah@rsa-sha:~/code/exa/searchdb$  time ./build/searchdb process --input=data/raw/ --output=data/processed/
[process] 100 docs processed
[process] 200 docs processed
[process] 300 docs processed
[process] 400 docs processed
[process] 500 docs processed
[process] 600 docs processed
[process] 700 docs processed
[process] 800 docs processed
[process] 900 docs processed
[process] done processed=986 skipped=14

real    0m8.145s
user    0m8.028s
sys     0m0.074s
```
</details>