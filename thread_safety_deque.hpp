#pragma once
#include <condition_variable>
#include <deque>
#include <initializer_list>
#include <mutex>
#include <utility>

template <typename T>
class ThreadSafetyDeque {
private:
	std::deque<T> _deque;
	std::condition_variable _condition;
	mutable std::mutex _mutex;

public:
	ThreadSafetyDeque() = default;

	ThreadSafetyDeque(std::initializer_list<T> values) {
		std::lock_guard<std::mutex> lock(_mutex);
		for (const auto& elem : values) {
			_deque.push_back(elem);
		}
	}

	void push_back(const T& value) {
		std::lock_guard<std::mutex> lock(_mutex);
		_deque.push_back(value);
		_condition.notify_one();
	}

	void push_back(T&& value) {
		std::lock_guard<std::mutex> lock(_mutex);
		_deque.push_back(std::move(value));
		_condition.notify_one();
	}

	void push_front(const T& value) {
		std::lock_guard<std::mutex> lock(_mutex);
		_deque.push_front(value);
		_condition.notify_one();
	}

	void push_front(T&& value) {
		std::lock_guard<std::mutex> lock(_mutex);
		_deque.push_front(std::move(value));
		_condition.notify_one();
	}

	T pop_back() {
		std::unique_lock<std::mutex> lock(_mutex);
		_condition.wait(lock, [this]() { return !_deque.empty(); });

		T value = std::move(_deque.back());
		_deque.pop_back();
		return value;
	}

	T pop_front() {
		std::unique_lock<std::mutex> lock(_mutex);
		_condition.wait(lock, [this]() { return !_deque.empty(); });

		T value = std::move(_deque.front());
		_deque.pop_front();
		return value;
	}

	size_t size() const {
		std::lock_guard<std::mutex> lock(_mutex);
		return _deque.size();
	}

	bool empty() const {
		std::lock_guard<std::mutex> lock(_mutex);
		return _deque.empty();
	}
};

TEST(ThreadSafetyDequeTest, MultithreadPushPopTest) {
    ThreadSafetyDeque<int> deque;

    constexpr int num_threads = 5;
    constexpr int elems_per_thread = 1000;
    std::atomic<int> push_count = 0;
    std::atomic<int> pop_count = 0;

    std::vector<std::thread> push_threads;
    for (int i = 0; i < num_threads; ++i) {
        push_threads.emplace_back([&deque, &push_count, i]() {
            for (int j = 0; j < elems_per_thread; ++j) {
                if (j % 2 == 0) {
                    deque.push_back(j + i * elems_per_thread);
                }

                else {
                    deque.push_front(j + i * elems_per_thread);
                }

                ++push_count;
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::vector<std::thread> pop_threads;
    for (int i = 0; i < num_threads; ++i) {
        pop_threads.emplace_back([&deque, &pop_count]() {
            for (int j = 0; j < elems_per_thread; ++j) {
                if (j % 2 == 0) {
                    deque.pop_front();
                }

                else {
                    deque.pop_back();
                }

                ++pop_count;
            }
        });
    }

    for (auto& t : push_threads) {
        t.join();
    }

    for (auto& t : pop_threads) {
        t.join();
    }

    EXPECT_EQ(push_count.load(), num_threads * elems_per_thread);
    EXPECT_EQ(pop_count.load(), num_threads * elems_per_thread);
    EXPECT_TRUE(deque.empty());
}
