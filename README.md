# ThreadPool-Cpp
This repository contains a C++20 implementation of a Thread Pool, using the most recent std::jthreads and implementing various extra functionalities!

## Documentation

### Constructors

- Thread_Pool(n_of_workers);

Instantiate a Thread Pool object with n_of_workers workers:

    Thread_Pool pool(9); // 9 workers

You can also instantiate a Thread Pool with 0 workers: it will just collect tasks in its queue until you change their numbers:

    Thread_Pool pool(0); // No workers

### Methods

- add_task(task);

Add a generic task to the Thread Pool queue (The method is thread safe, and can be called from any thread):

    Thread_Pool pool(5);

    pool.add_task([]() -> void {
        std::cout << "Hello Thread Pool!\n";
    });

If your task must return something, store the future returned by the add_task method, and call get():

    Thread_Pool pool(12);

    auto future = pool.add_task([]() -> std::string {
        return "Hello Thread Pool!\n";
    });

    std::cout << future.get();

- change_number_of_workers(n_of_workers)

Change the number of workers of the Thread Pool (The method is thread safe, and can be called from any thread):

    Thread_Pool pool(19);

    pool.change_number_of_workers(3);

If the new number of workers is the same as the current one, nothing happens:

    Thread_Pool pool(5);

    for (int i = 0; i != 20; ++i) {
        pool.add_task([]() -> void {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        });
    }

    pool.change_number_of_workers(5); // Nothing happens

If the new number of workers is greater than the current one, new workers will be added and will immediately start running tasks:

    Thread_Pool pool(5);

    for (int i = 0; i != 20; ++i) {
        pool.add_task([]() -> void {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        });
    }

    // 5 Tasks are now being executed, remaining 15 tasks waiting for free workers

    pool.change_number_of_workers(20); // The remaining 15 tasks are immediately captured by the new workers

If the new number of workers is less than the current one, the method will wait until the occupied workers that must be removed have finished their tasks:

    Thread_Pool pool(5);

    for (int i = 0; i != 20; ++i) {
        pool.add_task([]() -> void {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        });
    }

    // 5 Tasks are now being executed, remaining 15 tasks waiting for free workers

    pool.change_number_of_workers(1); // The Thread Pool is waiting for 4 of the 5 workers to be free, they are then removed from the Thread Pool

- flush_tasks(wait)

Flushes the entire queue, removing all pending tasks (The method is thread safe, and can be called from any thread).

The wait flag decides if you want to also wait on tasks that are already being executed by the workers (Default value: true):

    Thread_Pool pool1(7);

    for (int i = 0; i != 20; ++i) {
        pool1.add_task([]() -> void {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        });
    }

    pool1.flush_tasks(); // The 13 remaining tasks are removed from the queue: The method will also wait here until the already captured 7 tasks are completed

    Thread_Pool pool2(7);

    for (int i = 0; i != 20; ++i) {
        pool2.add_task([]() -> void {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        });
    }

    pool2.flush_tasks(false); // The 13 remaining tasks are removed from the queue: The method returns immediately, not waiting for the already captured 7 tasks to be completed

- wait()

The method will wait here until all tasks are executed (The method is thread safe, and can be called from any thread).

    Thread_Pool pool(1);

    for (int i = 0; i != 20; ++i) {
        pool.add_task([]() -> void {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        });
    }

    pool.wait(); // The method will wait here until all 20 tasks are executed

- set_idle_callback(callback)

A callback void function to be run each time the Thread Pool has no pending tasks and all workers are available (The method is thread safe, and can be called from any thread):

    Thread_Pool pool(8);

    for (int i = 0; i != 20; ++i) {
        pool.add_task([]() -> void {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        });
    }

    pool.set_idle_callback([]() -> void {
        std::cout << "Idling...\n";
    });

    // When all 20 tasks are executed, the callback is executed

- remove_idle_callback()

Removes the callback void function, and will not be called anymore (The method is thread safe, and can be called from any thread):

    Thread_Pool pool(8);

    for (int i = 0; i != 20; ++i) {
        pool.add_task([]() -> void {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        });
    }

    pool.set_idle_callback([]() -> void {
        std::cout << "Idling...\n";
    });

    pool.remove_idle_callback();

    // When all 20 tasks are executed, nothing happens
