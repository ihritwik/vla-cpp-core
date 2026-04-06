#include "onnx_inference.hpp"
#include <iostream>

// -----------------------------------------------------------------------------
// STUB IMPLEMENTATION
// All methods are unconditionally compiled as stubs because ONNX Runtime is
// not installed in this environment.  See the header for upgrade instructions.
// -----------------------------------------------------------------------------

ONNXInference::ONNXInference(const std::string& model_path)
    : model_path_(model_path) {
    std::cerr << "[ONNXInference] STUB MODE — model not loaded: "
              << model_path_ << "\n";
}

std::vector<float> ONNXInference::run(const std::vector<float>& /*input*/,
                                      std::size_t output_size) {
    return std::vector<float>(output_size, 0.0f);
}

std::string ONNXInference::get_input_name() const {
    return "input";
}

std::string ONNXInference::get_output_name() const {
    return "output";
}
