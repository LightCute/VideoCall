#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>

template<typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() : m_stopped(false) {}

    // 生产：推入数据
    void push(T item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_stopped) return;
        m_queue.push(std::move(item));
        m_cv.notify_one();
    }

    // 消费：弹出数据，如果队列为空且没停止则阻塞
    // 返回 false 表示队列已停止，应该退出线程
    bool pop(T& out) {
        std::unique_lock<std::mutex> lock(m_mutex);
        
        // 等待直到有数据 或者 被要求停止
        m_cv.wait(lock, [this]() {
            return !m_queue.empty() || m_stopped;
        });

        if (m_stopped && m_queue.empty()) {
            return false;
        }

        out = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    // 停止队列，唤醒所有等待的线程
    void stop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stopped = true;
        m_cv.notify_all();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<T> m_queue;
    bool m_stopped;
};