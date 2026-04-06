# action_decoder

> Logit-to-command translation for robot action execution.

## Purpose

Converts raw float logits from the VLA model output into a structured
`RobotCommand` that can be sent to a robot controller.  Supports both
discrete (classification) and continuous (regression) action spaces
with hard safety clamping on all output fields.

## `RobotCommand` Struct

```cpp
struct RobotCommand {
    float joint_angles[6];  // radians, range [-π, π]
    int   gripper_state;    // 0 = open, 1 = closed
    float velocity_scale;   // [0.0, 1.0]
};
```

| Field | Type | Range | Description |
|---|---|---|---|
| `joint_angles[6]` | `float[6]` | [-π, π] rad | Per-joint target angle |
| `gripper_state` | `int` | {0, 1} | 0 = open, 1 = closed |
| `velocity_scale` | `float` | [0.0, 1.0] | Motion speed multiplier |

## Discrete Mode — Action Index Table

| Index | Action | Effect |
|---|---|---|
| 0 | `MOVE_FORWARD` | joint[0] = +0.3 rad, vel = 0.7 |
| 1 | `MOVE_BACKWARD` | joint[0] = -0.3 rad, vel = 0.7 |
| 2 | `ROTATE_LEFT` | joint[1] = +0.5 rad, vel = 0.5 |
| 3 | `ROTATE_RIGHT` | joint[1] = -0.5 rad, vel = 0.5 |
| 4 | `GRIP_CLOSE` | gripper = 1 |
| 5 | `GRIP_OPEN` | gripper = 0 |
| 6 | `STOP` | vel = 0.0 |

Selection: `argmax(logits[0..6])`.

## Continuous Mode — Field Mapping Table

| Logit index | Output field | Transform |
|---|---|---|
| `logits[0]` | `joint_angles[0]` | clamp(v, -π, π) |
| `logits[1]` | `joint_angles[1]` | clamp(v, -π, π) |
| `logits[2]` | `joint_angles[2]` | clamp(v, -π, π) |
| `logits[3]` | `joint_angles[3]` | clamp(v, -π, π) |
| `logits[4]` | `joint_angles[4]` | clamp(v, -π, π) |
| `logits[5]` | `joint_angles[5]` | clamp(v, -π, π) |
| `logits[6]` | `gripper_state` | 1 if > 0.5, else 0 |
| `logits[7]` | `velocity_scale` | clamp(v, 0.0, 1.0) |

## Class API

```cpp
#include "action_decoder.hpp"

ActionDecoder dec(Mode::CONTINUOUS);
RobotCommand cmd = dec.decode(logits);
std::cout << dec.to_string(cmd);
```

| Method | Description |
|---|---|
| `ActionDecoder(Mode mode)` | Construct with `DISCRETE` or `CONTINUOUS` |
| `RobotCommand decode(const std::vector<float>& logits)` | Decode logits → command |
| `std::string to_string(const RobotCommand& cmd) const` | Human-readable dump |

## Safety Clamping

All output fields are clamped **after** decoding regardless of
input magnitude.  Passing `1e6f` or `-1e6f` logits will never produce
out-of-range joint angles or velocity values.

## Extension Guide

To add a new action type:

1. Add a new `case` to `decode_discrete()` in `action_decoder.cpp`.
2. Increase the `NUM_ACTIONS` constant if needed.
3. Document the new index in this table.
4. Add a unit test in `tests/unit/test_action_decoder.cpp`.
