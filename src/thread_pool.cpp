#include <cstddef>
#include <memory>
#include <signal.h>
#include <condition_variable>
#include <functional>
#include <unistd.h>
#include <vector>
#include <thread>
#include <cstdio>
#include "thread_pool.hpp"

ThreadPool::ThreadPool(int num_threads) {
    int i;

    interrupt = false;
    num_threads = num_threads > 0 ? num_threads : std::thread::hardware_concurrency(); 
#ifdef DEBUG
    printf("Creating %d threads\n", num_threads);
#endif
    for(i = 0; i < num_threads; i++) {
        threads.emplace_back(std::thread(&ThreadPool::thread_loop, this));
    }
}

void ThreadPool::queue_job(std::function<void()> job) {
#ifdef DEBUG
    printf("Adding job to threadpool queue.\n");
#endif
    queue_lock.lock();
    work_queue.push(job);
    queue_lock.unlock();
    notify_work.notify_one();
}

int ThreadPool::size() {
	return threads.size();
}

ThreadPool::~ThreadPool() {
#ifdef DEBUG
    printf("Tearing down threadpool, joining all threads.\n");
#endif
    interrupt = true;
    notify_work.notify_all();
    for(std::thread &thread : threads) {
        thread.join();
    }
    threads.clear();
    while(!work_queue.empty())
        work_queue.pop();
}

void ThreadPool::thread_loop() {
    std::function<void()> job;

    while(true) {
        {
            std::unique_lock<std::mutex> work_lock(queue_lock);
            notify_work.wait(work_lock, [this]{ 
                return !work_queue.empty() || interrupt;
            });

            if(interrupt)
                return;

            job = work_queue.front();
            work_queue.pop();
        }

        job();
    }
}
