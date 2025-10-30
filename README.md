# ThreadPool-Cpp
This repository contains a C++20 implementation of a Thread Pool, using the most recent std::jthreads and implementing various extra functionalities!

## Documentation

### Constructors

- Thread_Pool(n_of_threads);

Instantiate a Thread Pool object with n_of_threads workers:

    Thread_Pool pool(9); // 9 Workers

You can also instantiate a Thread Pool with 0 workers: it will just collect tasks in its queue until you change their numbers:

    Thread_Pool pool(0); // No workers

### Methods

- add_task(task);

Add a generic task to the Thread Pool queue:

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

