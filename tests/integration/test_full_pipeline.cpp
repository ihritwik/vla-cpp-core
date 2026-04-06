// End-to-end integration test — no test framework.
// Exits 0 on full pass, 1 on first failure.

#include "../../camera_pipeline/camera_capture.hpp"
#include "../../camera_pipeline/ring_buffer/ring_buffer.hpp"
#include "../../inference_engine/vision_encoder/vision_encoder.hpp"
#include "../../action_decoder/action_decoder.hpp"
#include "../../action_decoder/serial_publisher/serial_publisher.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

static void fail(int step, const std::string& reason) {
    std::cerr << "\u274c FAILED at Step " << step << ": " << reason << "\n";
    std::exit(1);
}

int main() {
    auto t_start = std::chrono::high_resolution_clock::now();
    constexpr float PI = 3.14159265358979323846f;

    // ── Step 1: Synthetic Frame Generation ───────────────────────────────────
    cv::Mat frame(480, 640, CV_8UC3);
    // Seed the global OpenCV RNG for reproducibility, then fill with randu.
    cv::theRNG().state = 42;
    cv::randu(frame, cv::Scalar(0), cv::Scalar(255));

    if (frame.data == nullptr)      fail(1, "frame.data is null");
    if (frame.total() != 640u * 480u) fail(1, "unexpected frame dimensions");
    std::cout << "[PASS] Step 1: Synthetic Frame Generation\n";

    // ── Step 2: Preprocessing ─────────────────────────────────────────────────
    CameraCapture cap(0);
    auto tensor = cap.preprocess(frame);

    if (tensor.size() != 224u * 224u * 3u)
        fail(2, "tensor size wrong: " + std::to_string(tensor.size()));
    if (!std::all_of(tensor.begin(), tensor.end(),
                     [](float v) { return v >= 0.0f && v <= 1.0f; }))
        fail(2, "tensor values out of [0,1]");
    std::cout << "[PASS] Step 2: Preprocessing\n";

    // ── Step 3: Ring Buffer Round-Trip ────────────────────────────────────────
    RingBuffer<std::vector<float>, 8> buf;
    if (!buf.push(tensor)) fail(3, "ring buffer push failed");

    std::vector<float> retrieved;
    if (!buf.pop(retrieved))            fail(3, "ring buffer pop failed");
    if (retrieved.size() != tensor.size()) fail(3, "retrieved size mismatch");

    // Spot-check 10 evenly-spaced indices
    std::size_t n = tensor.size();
    for (int k = 0; k < 10; ++k) {
        std::size_t idx = (k * n) / 10;
        if (std::abs(retrieved[idx] - tensor[idx]) > 1e-6f)
            fail(3, "data corruption at index " + std::to_string(idx));
    }
    std::cout << "[PASS] Step 3: Ring Buffer Round-Trip\n";

    // ── Step 4: Vision Encoding ───────────────────────────────────────────────
    DummyEncoder encoder;
    auto embedding = encoder.encode(retrieved);

    if (embedding.size() != 512u)
        fail(4, "embedding size wrong: " + std::to_string(embedding.size()));
    if (!std::all_of(embedding.begin(), embedding.end(),
                     [](float v) { return std::isfinite(v); }))
        fail(4, "non-finite value in embedding");
    std::cout << "[PASS] Step 4: Vision Encoding\n";

    // ── Step 5: Action Decoding ───────────────────────────────────────────────
    ActionDecoder decoder(Mode::CONTINUOUS);
    RobotCommand cmd = decoder.decode(embedding);

    for (int i = 0; i < 6; ++i) {
        if (cmd.joint_angles[i] < -PI || cmd.joint_angles[i] > PI)
            fail(5, "joint_angle[" + std::to_string(i) + "] out of range");
    }
    if (cmd.gripper_state != 0 && cmd.gripper_state != 1)
        fail(5, "invalid gripper_state: " + std::to_string(cmd.gripper_state));
    if (cmd.velocity_scale < 0.0f || cmd.velocity_scale > 1.0f)
        fail(5, "velocity_scale out of [0,1]");
    std::cout << "[PASS] Step 5: Action Decoding\n";

    // ── Step 6: Serial Publish & Verify ───────────────────────────────────────
    const std::string out_path = "/tmp/robot_cmd_integration.json";
    SerialPublisher pub(out_path);
    if (!pub.write(cmd)) fail(6, "SerialPublisher::write returned false");

    std::ifstream ifs(out_path);
    if (!ifs.is_open()) fail(6, "output file not found: " + out_path);

    std::string content((std::istreambuf_iterator<char>(ifs)),
                         std::istreambuf_iterator<char>());
    if (content.find("timestamp_ms")   == std::string::npos) fail(6, "missing timestamp_ms");
    if (content.find("joint_angles")   == std::string::npos) fail(6, "missing joint_angles");
    if (content.find("gripper_state")  == std::string::npos) fail(6, "missing gripper_state");
    if (content.find("velocity_scale") == std::string::npos) fail(6, "missing velocity_scale");
    std::cout << "[PASS] Step 6: Serial Publish & Verify\n";

    auto t_end = std::chrono::high_resolution_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          t_end - t_start).count();

    std::cout << "\u2705 Full pipeline integration test PASSED ("
              << elapsed_ms << "ms elapsed)\n";
    return 0;
}
