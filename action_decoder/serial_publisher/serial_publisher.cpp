#include "serial_publisher.hpp"
#include <chrono>
#include <fstream>
#include <sstream>

SerialPublisher::SerialPublisher(const std::string& output_path)
    : output_path_(output_path) {}

bool SerialPublisher::write(const RobotCommand& cmd) {
    // Timestamp in milliseconds since Unix epoch.
    using namespace std::chrono;
    auto now_ms = duration_cast<milliseconds>(
                      system_clock::now().time_since_epoch())
                      .count();

    std::ostringstream json;
    json << "{\n";
    json << "  \"timestamp_ms\": " << now_ms << ",\n";
    json << "  \"joint_angles\": [";
    for (int i = 0; i < 6; ++i) {
        json << cmd.joint_angles[i];
        if (i < 5) json << ", ";
    }
    json << "],\n";
    json << "  \"gripper_state\": " << cmd.gripper_state << ",\n";
    json << "  \"velocity_scale\": " << cmd.velocity_scale << "\n";
    json << "}\n";

    std::ofstream ofs(output_path_, std::ios::trunc);
    if (!ofs.is_open()) {
        return false;
    }
    ofs << json.str();
    return ofs.good();
}
