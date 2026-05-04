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