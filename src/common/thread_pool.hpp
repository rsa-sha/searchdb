#pragma once

#include <vector>
#include <deque>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstddef>

class ThreadPool {
public:
	explicit ThreadPool(size_t num_threads, size_t max_queue_size = 1024)
		:max_queue_size_(max_queue_size),stopping_(false)
	{
		workers_.reserve(num_threads);
		for (size_t i = 0; i<num_threads; i++) {
			workers_.emplace_back([this](std::stop_token st) {
				worker_loop(st);
			});
		}
	}

	~ThreadPool ();

	void submit(std::function<void()> task);  // For producer thread(s) to provide tasks to consumer
	void shutdown();

private:
	void worker_loop(std::stop_token);			// Each consumer thread runs this

	std::vector<std::jthread> workers_;
	std::deque<std::function<void()>> queue_;

	std::mutex mu_;
	std::condition_variable	not_empty_;			// Workers wait on this
	std::condition_variable not_full_;			// Submit waits on this

	std::atomic<bool> stopping_;
	size_t	max_queue_size_;
};
