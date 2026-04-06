# inference_engine

> Model-agnostic ONNX inference abstraction for edge deployment.

## Purpose

Wraps the ONNX Runtime C++ API behind a simple interface so that the
rest of the pipeline can be developed and tested without a physical
model file or a specific hardware backend.

**Current state:** stub mode (zero-filled outputs, no external
dependencies).

## Class API — `ONNXInference`

```cpp
#include "onnx_inference.hpp"
```

| Method | Signature | Description |
|---|---|---|
| Constructor | `ONNXInference(const std::string& model_path)` | Load (or stub) the model |
| `run` | `std::vector<float> run(const std::vector<float>& input, size_t output_size)` | Run inference |
| `get_input_name` | `std::string get_input_name() const` | First input node name |
| `get_output_name` | `std::string get_output_name() const` | First output node name |

### Stub mode (currently active)

`run()` returns `std::vector<float>(output_size, 0.0f)` without
loading any model or linking against ONNX Runtime.  All existing tests
pass in stub mode.

## How to Enable Real ONNX Runtime

### Step 1 — Download the ARM64 release

```bash
# From github.com/microsoft/onnxruntime/releases
wget https://github.com/microsoft/onnxruntime/releases/download/v1.18.1/\
onnxruntime-linux-aarch64-1.18.1.tgz
tar -xzf onnxruntime-linux-aarch64-1.18.1.tgz
export ONNXRUNTIME_ROOT=$PWD/onnxruntime-linux-aarch64-1.18.1
```

### Step 2 — Update `inference_engine/CMakeLists.txt`

```cmake
# Remove -DONNX_STUB=ON from cmake invocation, then add:
target_include_directories(onnx_inference PUBLIC
    ${ONNXRUNTIME_ROOT}/include
)
target_link_libraries(onnx_inference PUBLIC
    ${ONNXRUNTIME_ROOT}/lib/libonnxruntime.so
)
```

### Step 3 — Replace the stub in `onnx_inference.cpp`

Swap the stub body of `run()` with real `Ort::Session::Run()` calls
(see the comment block in `onnx_inference.cpp`).

## Export a Model to ONNX (Python / PyTorch)

```python
import torch, torchvision
model = torchvision.models.vit_b_16(pretrained=True).eval()
dummy = torch.randn(1, 3, 224, 224)
torch.onnx.export(model, dummy, "vision_encoder.onnx",
                  input_names=["input"], output_names=["output"],
                  opset_version=17)
```

## Supported Backends (when real model connected)

| Backend | Hardware | How to enable |
|---|---|---|
| CPU (default) | Any | Default; no extra config |
| TensorRT | NVIDIA Jetson Orin | `OrtTensorRTProviderOptions` |
| QNN | Snapdragon 8 Elite | `OrtQNNProviderOptions` |

## Hardware Target Notes

| Hardware | Architecture | Recommended ONNX backend |
|---|---|---|
| Raspberry Pi 5 | ARM64 (Cortex-A76) | CPU (NEON auto-vectorization) |
| Jetson Orin NX | ARM64 + Ampere GPU | TensorRT EP |
| Snapdragon 8 Elite | ARM64 + Hexagon NPU | QNN EP |
| x86_64 Ubuntu | x86_64 | CPU (AVX2) |
