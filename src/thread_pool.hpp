#include <signal.h>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#pragma once

class ThreadPool {
    private:
        std::queue<std::function<void()>> work_queue;
        std::vector<std::thread> threads;
        volatile sig_atomic_t interrupt;
        std::mutex queue_lock;
        std::condition_variable notify_work;
        void thread_loop();
    public:
        ThreadPool(int num_threads = -1);
        ~ThreadPool();
        void queue_job(std::function<void()>);
        int size();
};