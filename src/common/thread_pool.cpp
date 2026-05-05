#include "thread_pool.hpp"

void ThreadPool::worker_loop(std::stop_token st) {
	while (true) {
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(mu_);
			// Wait if shutdown in sent or task in queue
			not_empty_.wait(lock, [&]{
					return stopping_.load() || !queue_.empty();
					});

			// Exit condition
			if (stopping_.load() && queue_.empty())
				return;

			// work
			task = std::move(queue_.front());
			queue_.pop_front();

			// Notify producer that space is available
			not_full_.notify_one();
		}
		// task execution
		task();
	}
}

void ThreadPool::submit(std::function<void()> task) {
	{
		std::unique_lock<std::mutex> lock(mu_);
		// Wait till there's space in the queue
		not_full_.wait(lock, [&]{
				return stopping_.load() || queue_.size() < max_queue_size_;
				});
		if (stopping_.load())
			throw std::runtime_error("ThreadPool is shutting down");
		queue_.push_back(task);
	}
	not_empty_.notify_one();
}

void ThreadPool::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mu_);
        stopping_.store(true);
    }

    not_empty_.notify_all();
    not_full_.notify_all();

    // jthreads auto-join on destruction
	for (auto &t : workers_) {
        if (t.joinable())
            t.join();   // <-- CRITICAL
    }
}


ThreadPool::~ThreadPool () {
	shutdown();
	/*
	{
		std::lock_guard<std::mutex> lock(mu_);
		stopping_.store(true);
	}		

	not_empty_.notify_all();
	not_full_.notify_all();

	// wake all workers
	for (auto &t: workers_) {
		t.request_stop();
	}
	*/
}

