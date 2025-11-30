#include "pch.h"
#include "../ThreadSafetyDeque/thread_safety_deque.hpp"

#include <string>
#include <thread>
#include <vector>

TEST(ThreadSafetyDequeTest, InitializerListTest) {
    ThreadSafetyDeque<int> deque{ 1, 2, 3, 4, 5 };

    EXPECT_EQ(deque.size(), 5);
    EXPECT_FALSE(deque.empty());
}

TEST(ThreadSafetyDequeTest, PushBackAndPopBackTest) {
    ThreadSafetyDeque<int> deque;

    deque.push_back(1);
    deque.push_back(2);

    EXPECT_EQ(deque.pop_back(), 2);
    EXPECT_EQ(deque.pop_back(), 1);
}


TEST(ThreadSafetyDequeTest, SizeAndEmptyTest) {
    ThreadSafetyDeque<int> deque;

    EXPECT_TRUE(deque.empty());
    EXPECT_EQ(deque.size(), 0);

    deque.push_back(10);

    EXPECT_FALSE(deque.empty());
    EXPECT_EQ(deque.size(), 1);
}

TEST(ThreadSafetyDequeTest, MovePushTest) {
    ThreadSafetyDeque<std::string> deque;

    std::string str = "abc";
    deque.push_back(std::move(str));

    EXPECT_TRUE(str.empty());

    std::string result = deque.pop_back();
    EXPECT_EQ(result, "abc");
}

TEST(ThreadSafetyDequeTest, PushFrontAndPopFrontTest) {
    ThreadSafetyDeque<int> deque;

    deque.push_front(1);
    deque.push_front(2);
    deque.push_front(3);

    EXPECT_EQ(deque.pop_front(), 3);
    EXPECT_EQ(deque.pop_front(), 2);
    EXPECT_EQ(deque.pop_front(), 1);
}

TEST(ThreadSafetyDequeTest, MultithreadPushTest) {
    ThreadSafetyDeque<int> deque;

    constexpr int num_threads = 5;
    constexpr int elems_per_thread = 100;

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < elems_per_thread; ++j) {
                deque.push_back(i * elems_per_thread + j);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(deque.size(), num_threads * elems_per_thread);
}
