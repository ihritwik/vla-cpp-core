#pragma once

#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "std_msgs/msg/string.hpp"

#include "camera_capture.hpp"
#include "ring_buffer/ring_buffer.hpp"
#include "vision_encoder/vision_encoder.hpp"
#include "action_decoder.hpp"
#include "serial_publisher/serial_publisher.hpp"

/// @brief ROS2 Jazzy node that drives the full VLA inference pipeline.
///
/// Subscribes to /camera/image_raw (sensor_msgs/Image), runs each frame
/// through preprocessing → ring-buffer → vision encoding → action decoding,
/// then publishes a JSON command string to /robot/command (std_msgs/String).
///
/// Parameters:
///   - model_path    (string, default "")   — ONNX model path; empty → DummyEncoder
///   - embedding_dim (int,    default 512)  — output embedding dimension
///   - action_mode   (string, default "continuous") — "continuous" or "discrete"
///   - queue_size    (int,    default 10)   — subscription/publisher queue depth
class VlaInferenceNode : public rclcpp::Node {
public:
    /// @brief Constructs the node, declares parameters, creates pub/sub,
    ///        and initialises all pipeline components.
    explicit VlaInferenceNode(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

private:
    /// @brief Callback invoked for each incoming image message.
    ///
    /// Converts sensor_msgs/Image → cv::Mat → float tensor, pushes through
    /// the ring buffer, encodes, decodes, serialises to JSON, and publishes.
    ///
    /// @param msg Shared pointer to the incoming image message.
    void image_callback(const sensor_msgs::msg::Image::SharedPtr msg);

    // ── ROS2 interfaces ──────────────────────────────────────────────────────
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr       command_pub_;

    // ── Pipeline components ──────────────────────────────────────────────────
    CameraCapture                              camera_capture_;
    RingBuffer<std::vector<float>, 8>          ring_buffer_;
    std::unique_ptr<DummyEncoder>              dummy_encoder_;
    std::unique_ptr<VisionEncoder>             vision_encoder_;
    ActionDecoder                              action_decoder_;
    SerialPublisher                            serial_publisher_;

    // ── Runtime flags ────────────────────────────────────────────────────────
    bool use_dummy_encoder_;
};
