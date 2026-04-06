#include "action_decoder.hpp"
#include <algorithm>
#include <sstream>

ActionDecoder::ActionDecoder(Mode mode) : mode_(mode) {}

RobotCommand ActionDecoder::decode(const std::vector<float>& logits) {
    switch (mode_) {
        case Mode::DISCRETE:   return decode_discrete(logits);
        case Mode::CONTINUOUS: return decode_continuous(logits);
    }
    // Unreachable, but satisfies -Wreturn-type.
    return decode_continuous(logits);
}

// Discrete action index → predefined RobotCommand
// Index: 0=MOVE_FORWARD, 1=MOVE_BACKWARD, 2=ROTATE_LEFT, 3=ROTATE_RIGHT,
//        4=GRIP_CLOSE, 5=GRIP_OPEN, 6=STOP
RobotCommand ActionDecoder::decode_discrete(const std::vector<float>& logits) {
    constexpr int NUM_ACTIONS = 7;
    // Zero-pad if needed
    std::vector<float> padded(logits);
    if (padded.size() < static_cast<std::size_t>(NUM_ACTIONS)) {
        padded.resize(NUM_ACTIONS, 0.0f);
    }

    int action_idx = static_cast<int>(
        std::distance(padded.begin(),
                      std::max_element(padded.begin(),
                                       padded.begin() + NUM_ACTIONS)));

    RobotCommand cmd{};
    for (int i = 0; i < 6; ++i) cmd.joint_angles[i] = 0.0f;
    cmd.gripper_state  = 0;
    cmd.velocity_scale = 0.5f;

    switch (action_idx) {
        case 0:  // MOVE_FORWARD
            cmd.joint_angles[0] = 0.3f;
            cmd.velocity_scale  = 0.7f;
            break;
        case 1:  // MOVE_BACKWARD
            cmd.joint_angles[0] = -0.3f;
            cmd.velocity_scale  = 0.7f;
            break;
        case 2:  // ROTATE_LEFT
            cmd.joint_angles[1] = 0.5f;
            cmd.velocity_scale  = 0.5f;
            break;
        case 3:  // ROTATE_RIGHT
            cmd.joint_angles[1] = -0.5f;
            cmd.velocity_scale  = 0.5f;
            break;
        case 4:  // GRIP_CLOSE
            cmd.gripper_state = 1;
            break;
        case 5:  // GRIP_OPEN
            cmd.gripper_state = 0;
            break;
        case 6:  // STOP
        default:
            cmd.velocity_scale = 0.0f;
            break;
    }

    // Safety clamp
    for (int i = 0; i < 6; ++i) {
        cmd.joint_angles[i] = clamp(cmd.joint_angles[i], -PI, PI);
    }
    cmd.velocity_scale = clamp(cmd.velocity_scale, 0.0f, 1.0f);
    return cmd;
}

RobotCommand ActionDecoder::decode_continuous(const std::vector<float>& logits) {
    // Minimum 8 elements needed; zero-pad if shorter.
    std::vector<float> padded(logits);
    if (padded.size() < 8) {
        padded.resize(8, 0.0f);
    }

    RobotCommand cmd{};
    for (int i = 0; i < 6; ++i) {
        cmd.joint_angles[i] = clamp(padded[static_cast<std::size_t>(i)],
                                    -PI, PI);
    }
    cmd.gripper_state  = (padded[6] > 0.5f) ? 1 : 0;
    cmd.velocity_scale = clamp(padded[7], 0.0f, 1.0f);
    return cmd;
}

std::string ActionDecoder::to_string(const RobotCommand& cmd) const {
    std::ostringstream oss;
    oss << "RobotCommand{joints=[";
    for (int i = 0; i < 6; ++i) {
        oss << cmd.joint_angles[i];
        if (i < 5) oss << ", ";
    }
    oss << "], gripper=" << cmd.gripper_state
        << ", vel=" << cmd.velocity_scale << "}";
    return oss.str();
}
