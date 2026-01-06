// ThreadPool.h
#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool {
public:
    explicit ThreadPool(size_t workerCount);
    ~ThreadPool();

    // 向线程池投递任务
    void post(std::function<void()> task);

private:
    // 每个 worker 线程执行的函数
    void workerLoop();

private:
    std::vector<std::thread> workers_;          // 固定数量线程
    std::queue<std::function<void()>> tasks_;   // 任务队列

    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_ = false;
};
