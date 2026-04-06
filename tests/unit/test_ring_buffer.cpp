#include "../vendor/catch2.hpp"
#include "../../camera_pipeline/ring_buffer/ring_buffer.hpp"
#include <thread>
#include <atomic>
#include <numeric>

TEST_CASE("FIFO ordering", "[ring_buffer]") {
    RingBuffer<int, 16> buf;
    for (int i = 0; i < 10; ++i) {
        REQUIRE(buf.push(i));
    }
    for (int i = 0; i < 10; ++i) {
        int val = -1;
        REQUIRE(buf.pop(val));
        REQUIRE(val == i);
    }
}

TEST_CASE("Overflow returns false", "[ring_buffer]") {
    RingBuffer<int, 4> buf;
    REQUIRE(buf.push(1));
    REQUIRE(buf.push(2));
    REQUIRE(buf.push(3));
    REQUIRE(buf.push(4));
    REQUIRE(buf.full());
    REQUIRE_FALSE(buf.push(99));
}

TEST_CASE("Empty pop returns false", "[ring_buffer]") {
    RingBuffer<int, 8> buf;
    REQUIRE(buf.empty());
    int val = 0;
    REQUIRE_FALSE(buf.pop(val));
}

TEST_CASE("Concurrent push/pop SPSC", "[ring_buffer]") {
    RingBuffer<int, 128> buf;
    constexpr int N = 1000;
    std::atomic<long long> sum_produced{0};
    std::atomic<long long> sum_consumed{0};

    std::thread producer([&] {
        for (int i = 0; i < N; ++i) {
            while (!buf.push(i)) { /* spin */ }
            sum_produced += i;
        }
    });

    std::thread consumer([&] {
        int received = 0;
        int item     = 0;
        while (received < N) {
            if (buf.pop(item)) {
                sum_consumed += item;
                ++received;
            }
        }
    });

    producer.join();
    consumer.join();

    REQUIRE(sum_produced == sum_consumed);
}
