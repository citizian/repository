// include/ThreadPool.h
#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    // 禁止拷贝
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // 提交任务
    void enqueue(const std::function<void()>& task);

private:
    std::vector<std::thread> workers;          // 工作线程
    std::queue<std::function<void()>> tasks;   // 任务队列

    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;

    void workerThread();                        // 每个线程执行函数
};
