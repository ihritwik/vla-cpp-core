#include "../vendor/catch2.hpp"
#include "../../action_decoder/action_decoder.hpp"
#include "../../action_decoder/serial_publisher/serial_publisher.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

static const std::string TEST_PATH = "/tmp/test_robot_cmd.json";

// Helper: build a known RobotCommand
static RobotCommand make_cmd() {
    RobotCommand cmd{};
    cmd.joint_angles[0] = 0.1f;
    cmd.joint_angles[1] = -0.3f;
    cmd.joint_angles[2] = 0.5f;
    cmd.joint_angles[3] = 0.0f;
    cmd.joint_angles[4] = 1.2f;
    cmd.joint_angles[5] = -0.7f;
    cmd.gripper_state   = 1;
    cmd.velocity_scale  = 0.75f;
    return cmd;
}

TEST_CASE("File created after write", "[serial_publisher]") {
    SerialPublisher pub(TEST_PATH);
    REQUIRE(pub.write(make_cmd()));
    REQUIRE(std::filesystem::exists(TEST_PATH));
}

TEST_CASE("JSON has required keys", "[serial_publisher]") {
    SerialPublisher pub(TEST_PATH);
    pub.write(make_cmd());

    std::ifstream ifs(TEST_PATH);
    REQUIRE(ifs.is_open());
    std::string content((std::istreambuf_iterator<char>(ifs)),
                         std::istreambuf_iterator<char>());

    REQUIRE(content.find("timestamp_ms")  != std::string::npos);
    REQUIRE(content.find("joint_angles")  != std::string::npos);
    REQUIRE(content.find("gripper_state") != std::string::npos);
    REQUIRE(content.find("velocity_scale") != std::string::npos);
}

TEST_CASE("joint_angles array has 6 values", "[serial_publisher]") {
    SerialPublisher pub(TEST_PATH);
    pub.write(make_cmd());

    std::ifstream ifs(TEST_PATH);
    std::string content((std::istreambuf_iterator<char>(ifs)),
                         std::istreambuf_iterator<char>());

    // Find the joint_angles array and count commas inside it.
    auto start = content.find('[');
    auto end   = content.find(']', start);
    REQUIRE(start != std::string::npos);
    REQUIRE(end   != std::string::npos);

    std::string arr = content.substr(start + 1, end - start - 1);
    int commas = 0;
    for (char c : arr) {
        if (c == ',') ++commas;
    }
    REQUIRE(commas == 5);  // 5 commas == 6 values
}

TEST_CASE("Round-trip gripper_state integrity", "[serial_publisher]") {
    RobotCommand cmd = make_cmd();
    cmd.gripper_state = 1;

    SerialPublisher pub(TEST_PATH);
    pub.write(cmd);

    std::ifstream ifs(TEST_PATH);
    std::string content((std::istreambuf_iterator<char>(ifs)),
                         std::istreambuf_iterator<char>());

    // Find "gripper_state": X and parse X
    auto pos = content.find("\"gripper_state\":");
    REQUIRE(pos != std::string::npos);
    auto colon = content.find(':', pos);
    auto num_start = content.find_first_not_of(" \t\n\r", colon + 1);
    int parsed = std::stoi(content.substr(num_start));
    REQUIRE(parsed == 1);
}
