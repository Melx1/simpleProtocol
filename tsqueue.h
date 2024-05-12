#ifndef SIMPLEPROTOCOL_TSQUEUE_H
#define SIMPLEPROTOCOL_TSQUEUE_H

#include <mutex>
#include <queue>
#include <condition_variable>

//thread safe queue
template <typename T>
class TSQueue {
private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;

public:
    TSQueue() = default;
    TSQueue(const TSQueue& other) {
        std::lock_guard<std::mutex> lock (other.m_mutex);
        m_queue = other.m_queue;
    }

    void push(T item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(item);
        m_cond.notify_one();
    }

    void waitPop(T& value) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [this]() { return !m_queue.empty(); });
        value = m_queue.front();
        m_queue.pop();
    }

    std::shared_ptr<T> waitPop(){
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [this]() { return !m_queue.empty(); });
        std::shared_ptr<T> ptr (std::make_shared<T>(m_queue.front()));
        m_queue.pop();
        return ptr;
    }
};

template <>
class TSQueue<uint8_t>{
private:
    std::queue<uint8_t> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;

public:
    TSQueue() = default;
    TSQueue(const TSQueue& other) {
        std::lock_guard<std::mutex> lock (other.m_mutex);
        m_queue = other.m_queue;
    }

    void push(uint8_t item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(item);
        m_cond.notify_one();
    }

    void push(std::vector<uint8_t> items) { //using the ref may cause a race condition, need optimization
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto item : items) {
            m_queue.push(item);
        }
        m_cond.notify_one();
    }

    void waitPop(uint8_t& value) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [this]() { return !m_queue.empty(); });
        value = m_queue.front();
        m_queue.pop();
    }

    std::shared_ptr<uint8_t> waitPop(){
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [this]() { return !m_queue.empty(); });
        std::shared_ptr<uint8_t> ptr (std::make_shared<uint8_t>(m_queue.front()));
        m_queue.pop();
        return ptr;
    }
};

#endif //SIMPLEPROTOCOL_TSQUEUE_H
