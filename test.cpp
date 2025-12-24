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

TEST(ThreadSafetyDequeTest, MultithreadWaitEmptyTest) {
    ThreadSafetyDeque<int> deque;

    std::atomic<bool> consumer_finished = false;
    std::atomic<bool> producer_finished = false;

    std::thread consumer([&]() {
        int value = deque.pop_back();
        EXPECT_EQ(value, 5);
        consumer_finished = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::thread producer([&]() {
        deque.push_back(10);
        deque.push_back(20);
        deque.push_back(5);
        producer_finished = true;
    });

    consumer.join();
    producer.join();

    EXPECT_TRUE(consumer_finished.load());
    EXPECT_TRUE(producer_finished.load());
}
