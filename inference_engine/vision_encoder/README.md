# vision_encoder

> Vision embedding extraction for VLA action conditioning.

## Purpose

Provides the "V" (Vision) stage of the VLA pipeline by encoding
a preprocessed image tensor into a compact 512-dimensional embedding
vector that conditions the action decoder.

## Pipeline Diagram

```
[Raw Frame]  →  [CameraCapture::preprocess]  →  [VisionEncoder::encode]
     640×480           224×224×3 float32            512-dim float32
                                                          │
                                                     [Embedding]
                                                     [512-dim]
                                                          │
                                                [ActionDecoder::decode]
                                                          │
                                                   [RobotCommand]
```

## Class API

### `VisionEncoder` (production)

```cpp
#include "vision_encoder.hpp"
VisionEncoder enc("clip_vit_b32.onnx");
std::vector<float> embedding = enc.encode(tensor);  // tensor: 224*224*3
```

| Method | Signature | Description |
|---|---|---|
| Constructor | `VisionEncoder(const std::string& model_path)` | Load ONNX model |
| `encode` | `std::vector<float> encode(const std::vector<float>& tensor)` | Return 512-dim embedding |

### `DummyEncoder` (testing — no model required)

```cpp
DummyEncoder enc;
auto emb = enc.encode(tensor);   // deterministic, range [-1, 1]
```

Same API as `VisionEncoder`. Returns a reproducible 512-dim vector
seeded from the input checksum — identical input always yields
identical output.

## Embedding Configuration

| Property | Value |
|---|---|
| Output dimension | 512 |
| Value range (DummyEncoder) | [-1.0f, 1.0f] |
| Determinism | Yes (fixed seed derived from input content) |

## Supported Architectures

| Architecture | ONNX opset | Notes |
|---|---|---|
| CLIP ViT-B/32 | 17 | 224×224 input, 512-dim output |
| SigLIP (Google) | 17 | 224×224 input, 512-dim output |
| Custom CNN | Any | Change `output_size` in `encode()` |

## Export CLIP Encoder to ONNX

```python
import torch
from transformers import CLIPVisionModel

model = CLIPVisionModel.from_pretrained("openai/clip-vit-base-patch32")
model.eval()
dummy = torch.randn(1, 3, 224, 224)
torch.onnx.export(
    model, dummy, "clip_vision.onnx",
    input_names=["pixel_values"],
    output_names=["pooler_output"],
    opset_version=17,
)
```
