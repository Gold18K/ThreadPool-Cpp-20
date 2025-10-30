# ThreadPool-Cpp
This repository contains a C++20 implementation of a Thread Pool, using the most recent std::jthreads and implementing various extra functionalities!

## Documentation

### Constructors

- Thread_Pool(n_of_workers);

Instantiate a Thread Pool object with n_of_threads workers:

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

If the new number of workers is the same as the current one, nothing happens;

    Thread_Pool pool(5);

    for (int i = 0; i != 20; ++i) {
        pool.add_task([]() -> bool {
            return true;
        });
    }

    pool.change_number_of_workers(5); // Nothing happens

If the new number of workers is greater than the current one, new workers will be added and will immediately start running tasks;

    Thread_Pool pool(5);

    for (int i = 0; i != 20; ++i) {
        pool.add_task([]() -> bool {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return true;
        });
    }

    // 5 Tasks are now being executed, remaining 15 tasks waiting for free workers

    pool.change_number_of_workers(20); // The remaining 15 tasks are immediately captured by the new workers

If the new number of workers is less than the current one, the method will wait until the occupied workers that must be removed have finished their tasks;

    Thread_Pool pool(5);

    for (int i = 0; i != 20; ++i) {
        pool.add_task([]() -> bool {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return true;
        });
    }

    // 5 Tasks are now being executed, remaining 15 tasks waiting for free workers

    pool.change_number_of_workers(1); // The Thread Pool is waiting for 4 of the 5 workers to be free, they are then removed from the Thread Pool
