#pragma once

#include <string>
#include <vector>

// =============================================================================
// STUB MODE — ONNX Runtime is not installed.
//
// This wrapper compiles cleanly with zero external dependencies and returns
// correctly-shaped zero tensors from run().
//
// To swap in real ONNX Runtime:
//  1. Download the ARM64 release from
//     https://github.com/microsoft/onnxruntime/releases
//  2. Extract and set ONNXRUNTIME_ROOT in CMakeLists.txt.
//  3. Replace the stub implementation in onnx_inference.cpp with the
//     Ort::Session / Ort::RunOptions API.
//  4. Remove the ONNX_STUB compile definition from CMakeLists.txt.
// =============================================================================

/// @brief Model-agnostic ONNX inference wrapper.
///
/// In stub mode (ONNX_STUB defined) all methods return zero-filled tensors
/// of the requested size without loading any model file.
class ONNXInference {
public:
    /// @brief Constructs the inference wrapper and (stub: logs) the model path.
    /// @param model_path Path to the .onnx model file.
    explicit ONNXInference(const std::string& model_path);

    /// @brief Runs inference on the provided input tensor.
    ///
    /// Stub mode: returns std::vector<float>(output_size, 0.0f).
    ///
    /// @param input       Flat float input tensor.
    /// @param output_size Expected number of output elements.
    /// @return Flat float output tensor.
    std::vector<float> run(const std::vector<float>& input,
                           std::size_t               output_size);

    /// @brief Returns the first input node name (stub: "input").
    /// @return Input tensor name.
    std::string get_input_name() const;

    /// @brief Returns the first output node name (stub: "output").
    /// @return Output tensor name.
    std::string get_output_name() const;

private:
    std::string model_path_;
};
