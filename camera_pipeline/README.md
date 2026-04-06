# camera_pipeline

> Real-time frame capture and preprocessing for VLA inference pipelines.

## Purpose

Captures frames from a USB/CSI camera (or synthetic source) and
converts them into normalized float tensors ready for model inference.
The pipeline feeds into a lock-free ring buffer that decouples capture
latency from inference latency.

## Class API — `CameraCapture`

```cpp
#include "camera_capture.hpp"
```

| Method | Description |
|---|---|
| `CameraCapture(int device_index)` | Bind to OpenCV device index (0 = default webcam) |
| `bool open()` | Open the capture device; returns false on failure |
| `std::vector<float> capture_and_preprocess()` | Capture one frame and preprocess it |
| `std::vector<float> preprocess(const cv::Mat& frame)` | Preprocess an existing frame |
| `void release()` | Release the device |
| `bool is_open() const` | Return current open state |

## Input / Output Specification

| Property | Value |
|---|---|
| Input | Raw BGR frame from `cv::VideoCapture` (any resolution) |
| Resize | 224 × 224 pixels via `cv::resize` |
| Conversion | `cv::Mat::convertTo(CV_32F)` |
| Normalization | `pixel / 255.0f` |
| Output | `std::vector<float>` of size `224 * 224 * 3 = 150528` |
| Memory order | HWC (Height × Width × Channels, BGR) |

### Normalization formula

```
output[i] = raw_pixel[i] / 255.0f    →  range [0.0f, 1.0f]
```

## Usage

```cpp
CameraCapture cap(0);
if (!cap.open()) { /* handle error */ }

auto tensor = cap.capture_and_preprocess();
// tensor.size() == 150528  (224*224*3)

cap.release();
```

## Build Instructions

```bash
# From repo root
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target camera_capture camera_pipeline_test
./build/camera_pipeline/camera_pipeline_test
```

Requires OpenCV 4.6.0:

```bash
sudo apt-get install -y libopencv-dev
```

## Dependencies

| Dependency | Version | Install |
|---|---|---|
| OpenCV | 4.6.0+ | `sudo apt-get install libopencv-dev` |

## Known Limitations

- No hardware encoder support (V4L2 H.264, MIPI CSI-2) — frames are
  decoded in software via OpenCV's generic backend.
- `CameraCapture` does not support multi-camera synchronization.
- Frame rate is not capped; use the ring buffer or a fixed-interval
  thread to control inference rate.
