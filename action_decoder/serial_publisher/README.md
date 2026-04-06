# serial_publisher

> Hardware-abstracted command publishing for robot controllers.

## Purpose

Serializes a `RobotCommand` to a JSON file, simulating the output
channel that would write to a real serial port (`/dev/ttyUSB0`) or
CAN bus interface on production hardware.

## JSON Schema

```json
{
  "timestamp_ms": 1712150400000,
  "joint_angles": [0.1, -0.3, 0.5, 0.0, 1.2, -0.7],
  "gripper_state": 1,
  "velocity_scale": 0.75
}
```

| Field | Type | Description |
|---|---|---|
| `timestamp_ms` | `int64` | Unix epoch milliseconds (`system_clock`) |
| `joint_angles` | `float[6]` | Joint angles in radians, range [-π, π] |
| `gripper_state` | `int` | 0 = open, 1 = closed |
| `velocity_scale` | `float` | Motion speed multiplier [0.0, 1.0] |

## Class API

```cpp
#include "serial_publisher.hpp"

SerialPublisher pub("/tmp/robot_cmd.json");
bool ok = pub.write(cmd);
```

| Method | Description |
|---|---|
| `SerialPublisher(const std::string& output_path)` | Bind to output file path |
| `bool write(const RobotCommand& cmd)` | Serialize and write; returns false on I/O error |

## Replacing with a Real Serial Port (POSIX termios)

```cpp
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

int fd = open("/dev/ttyUSB0", O_WRONLY | O_NOCTTY);
struct termios tty{};
cfsetospeed(&tty, B115200);
tty.c_cflag |= (CLOCAL | CREAD | CS8);
tcsetattr(fd, TCSANOW, &tty);

// Then replace ofstream write with:
::write(fd, json_string.c_str(), json_string.size());
close(fd);
```

## Extending to CAN Bus

Replace the file write with a `socketcan` send:

```cpp
#include <linux/can.h>
#include <linux/can/raw.h>
// Pack joint_angles into CAN frames (8 bytes per frame × 6 frames)
// Send via sendto() on a CAN_RAW socket
```

## `cmd_reader` — Live Demo Monitoring

`cmd_reader` reads and pretty-prints the output JSON to stdout.
Useful for watching the pipeline output in a second terminal.

```bash
# Build
cmake --build build --target cmd_reader

# Watch live (poll every 100 ms)
watch -n 0.1 ./build/action_decoder/cmd_reader /tmp/robot_cmd.json

# Or read a specific path
./build/action_decoder/cmd_reader /tmp/robot_cmd_integration.json
```
