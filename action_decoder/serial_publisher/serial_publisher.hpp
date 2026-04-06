#pragma once

#include "../action_decoder.hpp"
#include <string>

/// @brief Serializes RobotCommand to JSON and writes it to a file, simulating
///        hardware serial communication.
///
/// JSON schema:
/// @code
/// {
///   "timestamp_ms": 1712150400000,
///   "joint_angles": [0.1, -0.3, 0.5, 0.0, 1.2, -0.7],
///   "gripper_state": 1,
///   "velocity_scale": 0.75
/// }
/// @endcode
///
/// To replace with real serial port, open a POSIX fd to /dev/ttyUSB0 and
/// write the JSON string using the termios API (see README.md).
class SerialPublisher {
public:
    /// @brief Constructs the publisher bound to an output file path.
    /// @param output_path Path where the JSON command will be written
    ///                    (e.g. "/tmp/robot_cmd.json").
    explicit SerialPublisher(const std::string& output_path);

    /// @brief Serializes cmd to JSON and writes/overwrites the output file.
    /// @param cmd The RobotCommand to publish.
    /// @return true if the write succeeded, false on I/O error.
    bool write(const RobotCommand& cmd);

    /// @brief Serializes cmd to a JSON string without writing to disk.
    ///        Used by the ROS2 node to publish over /robot/command.
    /// @param cmd The RobotCommand to serialize.
    /// @return JSON string in the same format as write().
    std::string to_json_string(const RobotCommand& cmd);

private:
    std::string output_path_;
};
