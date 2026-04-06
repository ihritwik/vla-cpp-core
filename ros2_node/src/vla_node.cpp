#include "vla_inference_node/vla_node.hpp"

#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>

VlaInferenceNode::VlaInferenceNode(const rclcpp::NodeOptions& options)
    : Node("vla_inference_node", options),
      camera_capture_(0),
      action_decoder_(Mode::CONTINUOUS),
      serial_publisher_("/tmp/robot_cmd.json"),
      use_dummy_encoder_(true)
{
    // ── Declare parameters ───────────────────────────────────────────────────
    this->declare_parameter("model_path",    "");
    this->declare_parameter("embedding_dim", 512);
    this->declare_parameter("action_mode",   "continuous");
    this->declare_parameter("queue_size",    10);

    // ── Read parameters ──────────────────────────────────────────────────────
    const std::string model_path  = this->get_parameter("model_path").as_string();
    const std::string action_mode = this->get_parameter("action_mode").as_string();
    const int         queue_size  = this->get_parameter("queue_size").as_int();

    // ── Select action mode ───────────────────────────────────────────────────
    Mode decode_mode = (action_mode == "discrete") ? Mode::DISCRETE : Mode::CONTINUOUS;
    action_decoder_  = ActionDecoder(decode_mode);

    // ── Select encoder ───────────────────────────────────────────────────────
    if (model_path.empty()) {
        dummy_encoder_   = std::make_unique<DummyEncoder>();
        use_dummy_encoder_ = true;
        RCLCPP_INFO(this->get_logger(),
                    "model_path is empty — using DummyEncoder (stub mode)");
    } else {
        vision_encoder_  = std::make_unique<VisionEncoder>(model_path);
        use_dummy_encoder_ = false;
        RCLCPP_INFO(this->get_logger(),
                    "Loaded ONNX model from: %s", model_path.c_str());
    }

    RCLCPP_INFO(this->get_logger(),
                "action_mode=%s  queue_size=%d", action_mode.c_str(), queue_size);

    // ── Create subscriber and publisher ──────────────────────────────────────
    image_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
        "/camera/image_raw",
        rclcpp::QoS(queue_size),
        [this](const sensor_msgs::msg::Image::SharedPtr msg) {
            this->image_callback(msg);
        });

    command_pub_ = this->create_publisher<std_msgs::msg::String>(
        "/robot/command",
        rclcpp::QoS(queue_size));

    RCLCPP_INFO(this->get_logger(), "VlaInferenceNode ready");
}

void VlaInferenceNode::image_callback(const sensor_msgs::msg::Image::SharedPtr msg)
{
    // ── 1. Convert sensor_msgs/Image → cv::Mat (BGR8) ───────────────────────
    cv_bridge::CvImagePtr cv_ptr;
    try {
        cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    } catch (const cv_bridge::Exception& e) {
        RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
        return;
    }

    // ── 2. Preprocess to 224×224 float tensor ───────────────────────────────
    std::vector<float> tensor = camera_capture_.preprocess(cv_ptr->image);
    if (tensor.empty()) {
        RCLCPP_WARN(this->get_logger(), "preprocess() returned empty tensor — skipping frame");
        return;
    }

    // ── 3. Push into ring buffer, pop back out ───────────────────────────────
    ring_buffer_.push(tensor);
    std::vector<float> buffered;
    if (!ring_buffer_.pop(buffered)) {
        RCLCPP_WARN(this->get_logger(), "ring_buffer pop failed — skipping frame");
        return;
    }

    // ── 4. Encode to embedding ───────────────────────────────────────────────
    std::vector<float> embedding;
    if (use_dummy_encoder_) {
        embedding = dummy_encoder_->encode(buffered);
    } else {
        embedding = vision_encoder_->encode(buffered);
    }

    // ── 5. Decode to RobotCommand ────────────────────────────────────────────
    RobotCommand cmd = action_decoder_.decode(embedding);

    // ── 6. Serialize to JSON string and publish ──────────────────────────────
    std::string json = serial_publisher_.to_json_string(cmd);

    std_msgs::msg::String out;
    out.data = json;
    command_pub_->publish(out);
}
