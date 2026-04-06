#include "rclcpp/rclcpp.hpp"
#include "vla_inference_node/vla_node.hpp"

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<VlaInferenceNode>());
    rclcpp::shutdown();
    return 0;
}
