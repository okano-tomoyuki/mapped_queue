#ifndef UTILITY_CONCURRENT_QUEUE_HPP
#define UTILITY_CONCURRENT_QUEUE_HPP

#include <mutex>
#include <queue>

template<typename T>
class ConcurrentQueue {
public:
    void push(T value) {
        std::unique_lock<std::mutex> lock(mtx_);
        queue_.push(value);
    }

    void emplace(T value) {
        std::unique_lock<std::mutex> lock(mtx_);
        queue_.emplace(value);
    }

    T front() {
        std::unique_lock<std::mutex> lock(mtx_);
        return queue_.front();
    }

    T back() {
        std::unique_lock<std::mutex> lock(mtx_);
        return queue_.back();
    }

    void pop() {
        std::unique_lock<std::mutex> lock(mtx_);
        queue_.pop();
    }

    bool try_pop(T& value) {
        std::unique_lock<std::mutex> lock(mtx_);
        if (queue_.empty()) {
            return false;
        }
        value = queue_.front();
        queue_.pop();
        return true;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }

private:
    mutable std::mutex mtx_;
    std::queue<T> queue_;
};

#endif  // UTILITY_CONCURRENT_QUEUE_HPP
