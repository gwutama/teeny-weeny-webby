#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t num_threads)
        : stop(false), max_queue_size(num_threads * 2) { // Queue can hold twice the number of threads
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] {
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] {
                        return this->stop || !this->tasks.empty();
                    });
                    if (this->stop && this->tasks.empty()) return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (tasks.size() >= max_queue_size) {
            return;
        }
        tasks.push(std::move(task));
    }
    condition.notify_one();
}

bool ThreadPool::is_full() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    return tasks.size() >= max_queue_size;
}
