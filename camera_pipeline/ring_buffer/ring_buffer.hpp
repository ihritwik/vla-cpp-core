#pragma once

#include <array>
#include <atomic>
#include <cstddef>

/// @brief Lock-free Single Producer Single Consumer (SPSC) circular buffer.
///
/// Uses std::atomic with acquire/release ordering — no mutex, no
/// priority inversion, O(1) push and pop.
///
/// @tparam T        Element type stored in the buffer.
/// @tparam Capacity Maximum number of elements. Must be >= 2.
template <typename T, std::size_t Capacity>
class RingBuffer {
    static_assert(Capacity >= 2, "RingBuffer capacity must be at least 2");

public:
    RingBuffer() : head_(0), tail_(0) {}

    // Non-copyable, non-movable — atomics are not copyable.
    RingBuffer(const RingBuffer&)            = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    /// @brief Pushes an item into the buffer (producer side).
    /// @param item Value to insert.
    /// @return true if the item was inserted; false if the buffer was full.
    bool push(const T& item) {
        const std::size_t current_tail = tail_.load(std::memory_order_relaxed);
        const std::size_t next_tail    = (current_tail + 1) % (Capacity + 1);
        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false;  // full
        }
        buffer_[current_tail] = item;
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    /// @brief Pops an item from the buffer (consumer side).
    /// @param item Reference that will receive the popped value.
    /// @return true if an item was available; false if the buffer was empty.
    bool pop(T& item) {
        const std::size_t current_head = head_.load(std::memory_order_relaxed);
        if (current_head == tail_.load(std::memory_order_acquire)) {
            return false;  // empty
        }
        item = buffer_[current_head];
        head_.store((current_head + 1) % (Capacity + 1),
                    std::memory_order_release);
        return true;
    }

    /// @brief Returns true if the buffer contains no elements.
    /// @return Empty state (snapshot).
    bool empty() const {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

    /// @brief Returns true if the buffer cannot accept more elements.
    /// @return Full state (snapshot).
    bool full() const {
        const std::size_t next_tail =
            (tail_.load(std::memory_order_acquire) + 1) % (Capacity + 1);
        return next_tail == head_.load(std::memory_order_acquire);
    }

private:
    // Internal capacity is Capacity+1 to distinguish full from empty.
    std::array<T, Capacity + 1> buffer_;
    std::atomic<std::size_t>    head_;
    std::atomic<std::size_t>    tail_;
};
