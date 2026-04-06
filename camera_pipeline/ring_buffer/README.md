# ring_buffer

> Lock-free inter-thread frame queue for real-time robotics pipelines.

## Purpose

`RingBuffer<T, Capacity>` is a header-only, templated, lock-free
Single Producer Single Consumer (SPSC) circular buffer used to pass
preprocessed image tensors from the camera capture thread to the
inference thread without mutex contention.

## Why Lock-Free?

| Property | Mutex Queue | RingBuffer (SPSC) |
|---|---|---|
| Latency | Unbounded (blocking) | O(1) always |
| Priority inversion | Possible | Impossible |
| CPU overhead | Context switch | Spin (cache-friendly) |
| Real-time safe | No | Yes |

Using `std::atomic` with `std::memory_order_acquire` / `release`
pairs guarantees visibility without fences on every load.

## Template API

```cpp
RingBuffer<T, Capacity> buf;
```

| Method | Description | Returns |
|---|---|---|
| `push(const T& item)` | Insert item (producer side) | `bool` — false if full |
| `pop(T& item)` | Remove item (consumer side) | `bool` — false if empty |
| `empty() const` | Test empty state | `bool` |
| `full() const` | Test full state | `bool` |

## Thread Safety

**SPSC only.** Exactly one thread must call `push` and exactly one
thread must call `pop`. Multiple concurrent producers or consumers
are **not** supported and will cause data races.

## Usage Example

```cpp
#include "ring_buffer.hpp"
#include <thread>
#include <vector>

RingBuffer<std::vector<float>, 8> buf;

std::thread producer([&] {
    for (int i = 0; i < 100; ++i) {
        std::vector<float> frame(224 * 224 * 3, 0.5f);
        while (!buf.push(frame)) { /* spin */ }
    }
});

std::thread consumer([&] {
    std::vector<float> frame;
    int received = 0;
    while (received < 100) {
        if (buf.pop(frame)) {
            // process frame
            ++received;
        }
    }
});

producer.join();
consumer.join();
```

## Performance

- `push` and `pop`: **O(1)** — single atomic load + store per call.
- Cache-friendly: head and tail are separate cache lines
  (through natural alignment on 64-byte boundaries).

## Limitations

- **SPSC only** — do not share the producer or consumer role across threads.
- Capacity must be `>= 2` (enforced by `static_assert`).
- Internal storage is `Capacity + 1` elements (sentinel slot).
