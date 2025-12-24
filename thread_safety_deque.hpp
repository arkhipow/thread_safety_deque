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
