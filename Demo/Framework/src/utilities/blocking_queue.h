#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

template<typename T>
class BlockingQueue {
public:

    BlockingQueue() : m_stop(false) {}
    ~BlockingQueue() { stop(); }
    
    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stop = true;
        }
        m_cv.notify_all();
    }

    void push(T item)
    {
        if (m_stop) return;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(std::move(item));
        }
        m_cv.notify_one();
    }

    T waitAndPop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_cv.wait(lock, [this] {
            return m_stop || !m_queue.empty();
        });

        if (m_stop)
        {
            return T{};
        }

        T item = std::move(m_queue.front());
        m_queue.pop();
        return item;
    }

private:

    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_stop;
};