
// Inclusions
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include "Thread_Pool.h"
#include "Thread_Safe_Cout.h"
#include <vector>

// Constructors
Thread_Pool::Thread_Pool(const uint32_t& _n_of_workers) : n_of_threads(_n_of_workers),
													      idle_callback(nullptr),
														  waiting_threads(n_of_threads),
														  threads(),
													      reduction_flags(_n_of_workers, false),
														  tasks(),
														  global_mutex(),
														  internal_mutex(),
														  workers_condition(),
														  flush_condition() {

	for (uint32_t i = 0; i < n_of_threads; ++i) {
		threads.push_back(std::jthread([this, i](std::stop_token _sToken) {
			this->worker_loop(_sToken, i);
		}));
	}

}

// Destructor
Thread_Pool::~Thread_Pool() {

	for (auto& worker : threads)
		worker.request_stop();

	workers_condition.notify_all();

	flush_tasks(false);

	for (auto& worker : threads)
		if (worker.joinable())
			worker.join();

}

// Public methods
void Thread_Pool::change_number_of_workers(const uint32_t& _n_of_workers) {
	std::lock_guard<std::mutex> global_lock(global_mutex);

	if (_n_of_workers == n_of_threads)
		return;

	if (_n_of_workers > n_of_threads) {

		for (uint32_t i = n_of_threads; i < _n_of_workers; ++i) {
			reduction_flags.push_back(false);

			threads.push_back(std::jthread([this, i](std::stop_token _sToken) {
				this->worker_loop(_sToken, i);
			}));
		}

		waiting_threads += _n_of_workers - n_of_threads;
	}

	if (_n_of_workers < n_of_threads) {

		{
			std::lock_guard<std::recursive_mutex> lock(internal_mutex);

			for (uint32_t i = _n_of_workers; i < n_of_threads; ++i)
				reduction_flags[i] = true;

		}

		workers_condition.notify_all();

		for (uint32_t i = _n_of_workers; i < n_of_threads; ++i)
			if (threads[i].joinable())
				threads[i].join();

		reduction_flags.resize(_n_of_workers);
		threads.resize(_n_of_workers);
	}

	n_of_threads = _n_of_workers;
}
void Thread_Pool::flush_tasks(const bool& _wait) {
	std::lock_guard<std::mutex>            global_lock(global_mutex);
	std::unique_lock<std::recursive_mutex> internal_lock(internal_mutex);

	while (!tasks.empty())
		tasks.pop();

	if (_wait) {

		flush_condition.wait(internal_lock, [this] {
			return waiting_threads == n_of_threads;
		});

	}

}
void Thread_Pool::wait() {
	std::lock_guard<std::mutex>            global_lock(global_mutex);
	std::unique_lock<std::recursive_mutex> internal_lock(internal_mutex);

	flush_condition.wait(internal_lock, [this] {
		return tasks.empty() && (waiting_threads == n_of_threads);
	});

}
void Thread_Pool::remove_idle_callback() {
	std::lock_guard<std::mutex>           global_lock(global_mutex);
	std::lock_guard<std::recursive_mutex> internal_lock(internal_mutex);

	idle_callback = nullptr;
}

// Private methods
std::function<void()> Thread_Pool::retrieve_task() {
	std::lock_guard<std::recursive_mutex> lock(internal_mutex);

	if (tasks.empty())
		return []{};

	std::function<void()> task = tasks.front();
	tasks.pop();

	return task;
}
bool                  Thread_Pool::is_idle() {
	std::lock_guard<std::recursive_mutex> lock(internal_mutex);

	return tasks.empty() && (waiting_threads == n_of_threads);
}
void                  Thread_Pool::worker_loop(std::stop_token _sToken,
											   const uint32_t& _reduction_flag_index) {

	while (true) {
		std::function<void()> task;

		{
			std::unique_lock<std::recursive_mutex> lock(internal_mutex);

			workers_condition.wait(lock, [this, &_sToken, &_reduction_flag_index] {
				return _sToken.stop_requested() || !tasks.empty() || reduction_flags[_reduction_flag_index];
			});

			--waiting_threads;

			if ((_sToken.stop_requested() && tasks.empty()) || reduction_flags[_reduction_flag_index])
				return;
			
			task = retrieve_task();
		}

		task();

		++waiting_threads;
		flush_condition.notify_one();

		if (idle_callback && is_idle())
			(*idle_callback)();

	}

}