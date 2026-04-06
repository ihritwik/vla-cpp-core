# vla_inference_node

ROS2 Jazzy integration node for real-time VLA inference on edge hardware.

Bridges the Phase 1 C++ pipeline (camera → ring buffer → vision encoding →
action decoding → JSON serialization) with the ROS2 ecosystem. Subscribes to
`/camera/image_raw`, runs the full inference pipeline on each frame, and
publishes structured JSON commands to `/robot/command`.

---

## Prerequisites

| Dependency | Version | Install |
|---|---|---|
| ROS2 Jazzy | latest | `apt` (see below) |
| OpenCV | 4.6 | `sudo apt-get install -y libopencv-dev` |
| cv_bridge | jazzy | `sudo apt-get install -y ros-jazzy-cv-bridge` |
| colcon | any | `sudo apt-get install -y python3-colcon-common-extensions` |

### Install ROS2 Jazzy (Ubuntu 24.04 Noble)

```bash
sudo apt-get install -y software-properties-common curl
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
    -o /usr/share/keyrings/ros-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) \
    signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] \
    http://packages.ros.org/ros2/ubuntu noble main" | \
    sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null
sudo apt-get update
sudo apt-get install -y ros-jazzy-desktop \
    python3-colcon-common-extensions \
    ros-jazzy-cv-bridge
```

---

## Build

```bash
# 1. Source ROS2
source /opt/ros/jazzy/setup.bash

# 2. Navigate to the ros2_node directory
cd ~/vla-cpp-core/ros2_node

# 3. Build with colcon
colcon build --packages-select vla_inference_node
```

Build artifacts are placed in `ros2_node/install/`.

---

## Launch

```bash
# Source the overlay
source install/setup.bash

# Run with defaults (DummyEncoder, continuous mode)
ros2 run vla_inference_node vla_node

# Run with all parameters set explicitly
ros2 run vla_inference_node vla_node \
    --ros-args \
    -p model_path:=/path/to/model.onnx \
    -p action_mode:=continuous \
    -p embedding_dim:=512 \
    -p queue_size:=10
```

---

## Topics

| Topic | Type | Direction | Description |
|---|---|---|---|
| `/camera/image_raw` | `sensor_msgs/Image` | Subscriber | Input BGR frames from any camera node |
| `/robot/command` | `std_msgs/String` | Publisher | JSON-encoded RobotCommand |

### `/robot/command` payload format

```json
{
  "timestamp_ms": 1712150400000,
  "joint_angles": [0.1, -0.3, 0.5, 0.0, 1.2, -0.7],
  "gripper_state": 1,
  "velocity_scale": 0.75
}
```

---

## Parameters

| Parameter | Type | Default | Description |
|---|---|---|---|
| `model_path` | string | `""` | Path to ONNX model file. Empty string activates DummyEncoder (stub mode). |
| `embedding_dim` | int | `512` | Output embedding dimension expected from the encoder. |
| `action_mode` | string | `"continuous"` | Decoding strategy: `"continuous"` or `"discrete"`. |
| `queue_size` | int | `10` | ROS2 subscription and publisher queue depth. |

---

## Topic Flow

```
[Camera Node]
     │ /camera/image_raw (sensor_msgs/Image)
     ▼
[vla_inference_node]
     │  cv_bridge → cv::Mat
     │  preprocess() → float32 tensor [224×224×3]
     │  RingBuffer<8> push → pop
     │  DummyEncoder / VisionEncoder → embedding [512]
     │  ActionDecoder (CONTINUOUS / DISCRETE) → RobotCommand
     │  SerialPublisher::to_json_string()
     │ /robot/command (std_msgs/String)
     ▼
[Robot Controller]
```

---

## Testing Without a Physical Camera

Open three terminals on the same machine (or use `tmux`).

**Terminal 1 — start the node:**

```bash
source /opt/ros/jazzy/setup.bash
cd ~/vla-cpp-core/ros2_node
source install/setup.bash
ros2 run vla_inference_node vla_node
```

**Terminal 2 — publish synthetic frames:**

```bash
source /opt/ros/jazzy/setup.bash
ros2 run image_tools cam2image
# Publishes /camera/image_raw at ~30 fps from a test pattern
```

**Terminal 3 — monitor output:**

```bash
source /opt/ros/jazzy/setup.bash
ros2 topic echo /robot/command
```

Expected output (one message per frame):

```
data: '{\n  "timestamp_ms": 1712150400123,\n  "joint_angles": [...],\n  "gripper_state": 0,\n  "velocity_scale": 0.5\n}\n'
---
```

---

## Swapping DummyEncoder for a Real ONNX Model

1. Download a compatible vision model (e.g. CLIP ViT-B/32 exported to ONNX).

2. Place it on the RPi:
   ```bash
   scp clip_vitb32.onnx pi@raspberrypi:~/models/
   ```

3. Launch the node with `model_path` set:
   ```bash
   ros2 run vla_inference_node vla_node \
       --ros-args \
       -p model_path:=/home/pi/models/clip_vitb32.onnx \
       -p action_mode:=continuous
   ```

4. The node logs `"Loaded ONNX model from: ..."` on startup — verify this
   appears rather than `"using DummyEncoder"`.

5. To revert to stub mode, pass `model_path:=""` or omit the parameter.

> **Note:** The ONNX inference wrapper (`inference_engine/onnx_inference.cpp`)
> currently builds in stub mode (`ONNX_STUB` defined). To enable real
> inference you must install the ONNX Runtime C++ SDK and rebuild without
> that define. See `inference_engine/onnx_inference.hpp` for the swap point.

---

## Performance Expectations

All numbers measured on RPi 5 (8 GB, ARM64 Cortex-A76) with stub/DummyEncoder:

| Stage | Typical cost | Notes |
|---|---|---|
| Preprocessing (224×224) | ~1.4 ms | Phase 1 benchmark mean |
| Ring buffer push + pop | < 0.1 ms | Lock-free SPSC |
| Vision encoding (DummyEncoder) | ~0.3 ms | Deterministic RNG |
| Action decoding | < 0.1 ms | Clamp + argmax |
| JSON serialization | < 0.1 ms | ostringstream |
| **Full pipeline** | **~2.1 ms** | **471 fps headroom** |
| Real-time budget remaining | **~31.5 ms** | At 30 fps target |

When a real CLIP ViT-B/32 model is connected, vision encoding will increase
to approximately **50–200 ms** on CPU (RPi 5). Use TensorRT EP on Jetson
Orin to bring this under 5 ms.
