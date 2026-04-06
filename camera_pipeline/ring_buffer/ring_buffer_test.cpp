// Standalone smoke-test for RingBuffer — not part of the Catch2 suite.
// Compile: g++ -std=c++17 -Wall ring_buffer_test.cpp -o ring_buffer_test
// Run:     ./ring_buffer_test

#include "ring_buffer.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    // --- Basic push/pop ---
    RingBuffer<int, 4> buf;
    assert(buf.empty());
    assert(!buf.full());

    assert(buf.push(10));
    assert(buf.push(20));
    assert(buf.push(30));
    assert(buf.push(40));
    assert(buf.full());
    assert(!buf.push(99));  // must fail — buffer full

    int val = 0;
    assert(buf.pop(val) && val == 10);
    assert(buf.pop(val) && val == 20);
    assert(buf.pop(val) && val == 30);
    assert(buf.pop(val) && val == 40);
    assert(buf.empty());
    assert(!buf.pop(val));  // must fail — buffer empty

    // --- Concurrent SPSC ---
    RingBuffer<int, 128> spsc;
    constexpr int N = 10000;
    std::atomic<long long> sum_produced{0}, sum_consumed{0};

    std::thread producer([&] {
        for (int i = 0; i < N; ++i) {
            while (!spsc.push(i)) { /* spin */ }
            sum_produced += i;
        }
    });

    std::thread consumer([&] {
        int received = 0;
        int item     = 0;
        while (received < N) {
            if (spsc.pop(item)) {
                sum_consumed += item;
                ++received;
            }
        }
    });

    producer.join();
    consumer.join();
    assert(sum_produced == sum_consumed);

    std::cout << "All RingBuffer tests passed.\n";
    return 0;
}
