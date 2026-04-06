# vla-cpp-core

### Modular C++ VLA Inference Stack for Edge Robotics

![CI](https://github.com/ihritwik/vla-cpp-core/actions/workflows/ci.yml/badge.svg)
![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![C++17](https://img.shields.io/badge/C++-17-blue.svg)
![Platform](https://img.shields.io/badge/Platform-RPi5%20%7C%20Jetson%20%7C%20Snapdragon-green.svg)
![Ubuntu](https://img.shields.io/badge/Ubuntu-24.04-orange.svg)

---

## Overview

`vla-cpp-core` is a production-ready, real-time VLA (Vision-Language-Action)
inference stack written in modern C++17, designed for edge deployment on
resource-constrained ARM64 hardware such as the Raspberry Pi 5, NVIDIA Jetson
Orin, and Qualcomm Snapdragon 8 Elite.  The stack bridges vision-language
model outputs with physical robot actuation through a fully modular pipeline:
from raw camera frames to structured joint commands, with lock-free buffering,
deterministic encoding, and JSON-serialized command publishing — all without
runtime memory allocation in the hot path.

---

## Full Pipeline Architecture

```
┌──────────────┐   ┌─────────────┐   ┌──────────────┐
│    Camera    │──▶│    Ring     │──▶│   Vision     │
│   Capture    │   │   Buffer    │   │   Encoder    │
└──────────────┘   └─────────────┘   └──────┬───────┘
  640×480 BGR        lock-free SPSC         │
  → 224×224 f32      RingBuffer<8>     [Embedding]
                                       [512-dim]
                                            │
┌──────────────┐   ┌─────────────┐   ┌──────▼───────┐
│    Serial    │◀──│   Action    │◀──│   Action     │
│   Publisher  │   │   Decoder   │   │   Logits     │
└──────┬───────┘   └─────────────┘   └──────────────┘
       │             DISCRETE or
       ▼             CONTINUOUS
 /tmp/robot_cmd.json
 (swap for serial/CAN)
```

---

## Module Table

| # | Module | Path | Description |
|---|---|---|---|
| 1 | Camera Pipeline | `camera_pipeline/` | OpenCV capture → float32 tensor |
| 2 | Ring Buffer | `camera_pipeline/ring_buffer/` | Lock-free SPSC inter-thread queue |
| 3 | ONNX Inference | `inference_engine/` | Model-agnostic ONNX wrapper (stub mode) |
| 4 | Vision Encoder | `inference_engine/vision_encoder/` | 512-dim image embedding extraction |
| 5 | Action Decoder | `action_decoder/` | Logit → RobotCommand (discrete + continuous) |
| 6 | Serial Publisher | `action_decoder/serial_publisher/` | JSON command serialization to file/serial |

---

## Hardware Targets

| Hardware | Architecture | ONNX Backend | Status |
|---|---|---|---|
| Raspberry Pi 5 (8 GB) | ARM64 (Cortex-A76) | CPU stub | ✅ Primary dev target |
| NVIDIA Jetson Orin NX | ARM64 + Ampere | TensorRT EP | Planned |
| Snapdragon 8 Elite | ARM64 + Hexagon NPU | QNN EP | Planned |
| x86_64 Ubuntu | x86_64 | CPU | ✅ CI runner |

---

## Quick Start

```bash
git clone https://github.com/ihritwik/vla-cpp-core
cd vla-cpp-core

# Install system dependencies (Ubuntu 24.04)
sudo apt-get install -y build-essential cmake libopencv-dev curl

# Download Catch2 test header
mkdir -p tests/vendor
curl -L https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp \
     -o tests/vendor/catch2.hpp

# Configure and build
cmake -B build \
  -DBUILD_TESTS=ON \
  -DBUILD_BENCHMARKS=ON \
  -DONNX_STUB=ON \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build --parallel 4
```

---

## ROS2 Jazzy Node

The `ros2_node/` package integrates the full Phase 1 pipeline into a ROS2 Jazzy node.

```bash
# Install ROS2 Jazzy + cv_bridge (Ubuntu 24.04)
sudo apt-get install -y ros-jazzy-desktop python3-colcon-common-extensions ros-jazzy-cv-bridge

# Build
source /opt/ros/jazzy/setup.bash
cd ros2_node
colcon build --packages-select vla_inference_node

# Run (stub mode — DummyEncoder)
source install/setup.bash
ros2 run vla_inference_node vla_node

# Run with a real ONNX model
ros2 run vla_inference_node vla_node \
    --ros-args \
    -p model_path:=/path/to/model.onnx \
    -p action_mode:=continuous \
    -p embedding_dim:=512
```

Topics: `/camera/image_raw` (sub) → `/robot/command` (pub, JSON string).

See [ros2_node/README.md](ros2_node/README.md) for full documentation.

---

## Running Tests

### Unit tests

```bash
cd build
ctest --output-on-failure --verbose
```

Expected output: 6 test executables, all passing.

### Integration test

```bash
./build/tests/integration/test_full_pipeline
```

Expected output:

```
[PASS] Step 1: Synthetic Frame Generation
[PASS] Step 2: Preprocessing
[PASS] Step 3: Ring Buffer Round-Trip
[PASS] Step 4: Vision Encoding
[PASS] Step 5: Action Decoding
[PASS] Step 6: Serial Publish & Verify
✅ Full pipeline integration test PASSED (Xms elapsed)
```

---

## Running Benchmarks

```bash
./build/benchmarks/benchmark_pipeline
# Results saved to: benchmarks/results/benchmark_results.txt
```

---

## Real-Time Budget

Target: **33.33 ms per frame** (30 fps).

| Stage | Typical cost (RPi 5, stub mode) | Budget share |
|---|---|---|
| Preprocessing (224×224) | ~3–5 ms | 15% |
| Ring buffer push+pop | < 0.1 ms | < 1% |
| Vision encoding (DummyEncoder) | < 0.5 ms | 1.5% |
| Action decoding | < 0.1 ms | < 1% |
| Serial publish | ~0.5–2 ms | 6% |
| **Full pipeline** | **~5–8 ms** | **~25%** |

With a real CLIP model the vision encoding stage will dominate
(10–20 ms on CPU); TensorRT on Jetson brings this to < 5 ms.

---

## Roadmap

- [x] Camera capture and preprocessing pipeline
- [x] Lock-free ring buffer (SPSC)
- [x] ONNX inference wrapper (stub mode)
- [x] Vision encoder interface (DummyEncoder + VisionEncoder)
- [x] Action decoder (discrete + continuous)
- [x] Serial command publisher (JSON)
- [x] Unit tests (Catch2 v2)
- [x] End-to-end integration test
- [x] Latency benchmarking with stats table
- [x] GitHub Actions CI (ubuntu-24.04)
- [ ] Real ONNX model integration (CLIP ViT-B/32, OpenVLA)
- [ ] TensorRT EP backend (Jetson Orin)
- [ ] QNN EP backend (Snapdragon 8 Elite)
- [ ] CAN bus publisher
- [x] ROS2 Jazzy node (Ubuntu 24.04)
- [ ] INT8 / FP16 quantization support

---

## Contributing

1. Fork the repository.
2. Create a feature branch: `git checkout -b feature/my-change`.
3. Commit with a descriptive message.
4. Open a pull request against `main`.

All PRs must pass the full CI pipeline (build → unit tests →
integration test) before review.

---

## License

MIT
