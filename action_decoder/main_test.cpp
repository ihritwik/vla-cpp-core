// Standalone smoke-test for ActionDecoder and SerialPublisher.
// Compile with: see action_decoder/CMakeLists.txt
// Run:          ./action_decoder_test

#include "action_decoder.hpp"
#include "serial_publisher/serial_publisher.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

int main() {
    constexpr float PI = 3.14159265358979323846f;

    // --- Discrete mode ---
    {
        ActionDecoder dec(Mode::DISCRETE);
        std::vector<float> logits(7, 0.0f);
        logits[4] = 1.0f;  // GRIP_CLOSE
        auto cmd = dec.decode(logits);
        assert(cmd.gripper_state == 1);
        for (int i = 0; i < 6; ++i) {
            assert(cmd.joint_angles[i] >= -PI && cmd.joint_angles[i] <= PI);
        }
        assert(cmd.velocity_scale >= 0.0f && cmd.velocity_scale <= 1.0f);
        std::cout << "[PASS] Discrete decode: " << dec.to_string(cmd) << "\n";
    }

    // --- Continuous mode ---
    {
        ActionDecoder dec(Mode::CONTINUOUS);
        std::vector<float> logits = {0.1f, -0.3f, 0.5f, 0.0f, 1.2f, -0.7f,
                                     0.9f, 0.75f};
        auto cmd = dec.decode(logits);
        for (int i = 0; i < 6; ++i) {
            assert(cmd.joint_angles[i] >= -PI && cmd.joint_angles[i] <= PI);
        }
        assert(cmd.gripper_state == 1);
        assert(cmd.velocity_scale >= 0.0f && cmd.velocity_scale <= 1.0f);
        std::cout << "[PASS] Continuous decode: " << dec.to_string(cmd) << "\n";
    }

    // --- SerialPublisher ---
    {
        RobotCommand cmd{};
        cmd.joint_angles[0] = 0.1f;
        cmd.gripper_state   = 1;
        cmd.velocity_scale  = 0.5f;

        SerialPublisher pub("/tmp/test_robot_cmd_smoke.json");
        assert(pub.write(cmd));

        std::ifstream ifs("/tmp/test_robot_cmd_smoke.json");
        assert(ifs.is_open());
        std::string content((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());
        assert(content.find("timestamp_ms") != std::string::npos);
        assert(content.find("joint_angles") != std::string::npos);
        std::cout << "[PASS] SerialPublisher wrote valid JSON.\n";
    }

    std::cout << "All action_decoder tests passed.\n";
    return 0;
}
