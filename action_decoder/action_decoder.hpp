#pragma once

#include <string>
#include <vector>

/// @brief Six-DOF robot command produced by ActionDecoder.
struct RobotCommand {
    float joint_angles[6];  ///< Joint angles in radians, range [-π, π].
    int   gripper_state;    ///< 0 = open, 1 = closed.
    float velocity_scale;   ///< Velocity multiplier, range [0.0, 1.0].
};

/// @brief Selects the decoding strategy applied to raw logits.
enum class Mode {
    DISCRETE,   ///< Argmax over 7 action classes.
    CONTINUOUS  ///< Direct mapping of logits to joint/gripper/velocity fields.
};

/// @brief Converts raw VLA output logits into structured RobotCommand values.
///
/// Discrete mode maps the argmax of the first 7 logits to a named action
/// (MOVE_FORWARD, MOVE_BACKWARD, ROTATE_LEFT, ROTATE_RIGHT, GRIP_CLOSE,
///  GRIP_OPEN, STOP).
///
/// Continuous mode maps:
///   logits[0..5] → joint_angles (clamped to [-π, π])
///   logits[6]    → gripper_state (1 if > 0.5, else 0)
///   logits[7]    → velocity_scale (clamped to [0.0, 1.0])
///
/// All values are clamped to their valid ranges before returning.
class ActionDecoder {
public:
    /// @brief Constructs the decoder with the specified action space mode.
    /// @param mode DISCRETE or CONTINUOUS.
    explicit ActionDecoder(Mode mode);

    /// @brief Decodes a logit vector into a RobotCommand.
    ///
    /// Logit vectors shorter than required are zero-padded internally.
    ///
    /// @param logits Raw float logits from the VLA model output.
    /// @return Fully populated and safety-clamped RobotCommand.
    RobotCommand decode(const std::vector<float>& logits);

    /// @brief Formats a RobotCommand as a human-readable string.
    /// @param cmd Command to format.
    /// @return Multi-field string representation.
    std::string to_string(const RobotCommand& cmd) const;

private:
    Mode mode_;

    // 3.14159265358979323846 from C99 math — written as literal for
    // strict C++17 compliance without relying on M_PI POSIX extension.
    static constexpr float PI = 3.14159265358979323846f;

    RobotCommand decode_discrete(const std::vector<float>& logits);
    RobotCommand decode_continuous(const std::vector<float>& logits);

    static float clamp(float v, float lo, float hi) {
        return v < lo ? lo : (v > hi ? hi : v);
    }
};
