#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

// Очередь шаблонная, чтобы можно было передавать любые данные
template <typename T> class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() = default;

    // Очередь не должна копироваться
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
        }
        cond_var_.notify_one();
    }

    void wait_and_pop(T& value) {
        // Для condition_variable обязательно нужен unique_lock,
        // так как он умеет временно отпускать мьютекс во время ожидания
        std::unique_lock<std::mutex> lock(mutex_);

        // Поток засыпает и отпускает мьютекс, пока очередь пуста.
        // Когда кто-то вызовет notify_one(), поток проснется,
        // снова захватит мьютекс и проверит условие в лямбде.
        cond_var_.wait(lock, [this] { return !queue_.empty(); });

        value = std::move(queue_.front());
        queue_.pop();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_var_;
};
