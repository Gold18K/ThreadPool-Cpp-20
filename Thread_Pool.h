#pragma once

// Inclusions
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class Thread_Pool {

public:

	// Constructors
	Thread_Pool(const uint32_t& _n_of_workers);
	Thread_Pool(const Thread_Pool&) = delete;
	Thread_Pool(Thread_Pool&&)      = delete;

	// Destructor
	~Thread_Pool();

	// Copy and move operators
	Thread_Pool& operator=(const Thread_Pool&) = delete;
	Thread_Pool& operator=(Thread_Pool&&)      = delete;

	// Public methods
	template<typename F>
	requires std::invocable<F>
	auto add_task(F&& _task) -> std::future<std::invoke_result_t<F>> {
		std::lock_guard<std::mutex> global_lock(global_mutex);

		using return_type = std::invoke_result_t<F>;

		auto packaged_task = std::make_shared<std::packaged_task<return_type()>>(
			std::forward<F>(_task)
		);

		std::future<return_type> result_future = packaged_task->get_future();
		
		{
			std::lock_guard<std::recursive_mutex> internal_lock(internal_mutex);

			tasks.emplace([packaged_task]() {
				(*packaged_task)();
			});
		}

		workers_condition.notify_one();

		return result_future;
	}
	void change_number_of_workers(const uint32_t& _n_of_workers);
	void flush_tasks(const bool& _wait = true);
	void wait();
	template<typename F>
	requires std::invocable<F>
	void set_idle_callback(F&& _idle_callback) {
		std::lock_guard<std::mutex>           global_lock(global_mutex);
		std::lock_guard<std::recursive_mutex> internal_lock(internal_mutex);

		idle_callback = std::make_unique<std::function<void()>>(std::forward<F>(_idle_callback));
	}
	void remove_idle_callback();

private:

	// Private methods
	std::function<void()> retrieve_task();
	bool                  is_idle();
	void                  worker_loop(std::stop_token _sToken,
		                  			  const uint32_t& _reduction_flag_index);

	// Fields
	uint32_t 					           n_of_threads;
	std::unique_ptr<std::function<void()>> idle_callback;
	std::atomic<uint32_t>				   waiting_threads;
	std::vector<std::jthread>			   threads;
	std::vector<bool>					   reduction_flags;
	std::queue<std::function<void()>>	   tasks;
	std::mutex                             global_mutex;
	std::recursive_mutex				   internal_mutex;
	std::condition_variable_any			   workers_condition;
	std::condition_variable_any            flush_condition;

};