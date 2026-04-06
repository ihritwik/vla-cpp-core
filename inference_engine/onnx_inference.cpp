#include "onnx_inference.hpp"
#include <iostream>

// =============================================================================
#ifdef ONNX_STUB
// ── Stub implementation ───────────────────────────────────────────────────────

ONNXInference::ONNXInference(const std::string& model_path)
    : model_path_(model_path) {
    std::cerr << "[ONNXInference] STUB MODE — model not loaded: "
              << model_path_ << "\n";
}

std::vector<float> ONNXInference::run(const std::vector<float>& /*input*/,
                                      std::size_t output_size) {
    return std::vector<float>(output_size, 0.0f);
}

std::string ONNXInference::get_input_name()  const { return "input";  }
std::string ONNXInference::get_output_name() const { return "output"; }

// =============================================================================
#else
// ── Real ONNX Runtime implementation ─────────────────────────────────────────

namespace {
/// @brief Builds a SessionOptions with 4 intra-op threads and full graph opt.
Ort::SessionOptions make_session_options() {
    Ort::SessionOptions opts;
    opts.SetIntraOpNumThreads(4);
    opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    return opts;
}
} // namespace

ONNXInference::ONNXInference(const std::string& model_path,
                             std::size_t         output_size)
    : env_(ORT_LOGGING_LEVEL_WARNING, "vla_inference"),
      session_options_(make_session_options()),
      session_(env_, model_path.c_str(), session_options_),
      output_size_(output_size),
      input_shape_({1, 3, 224, 224})
{
    // Query tensor names directly from the model metadata.
    auto in_name  = session_.GetInputNameAllocated(0, allocator_);
    auto out_name = session_.GetOutputNameAllocated(0, allocator_);
    input_name_  = std::string(in_name.get());
    output_name_ = std::string(out_name.get());

    std::cerr << "[ONNXInference] Loaded model: " << model_path
              << "  input=" << input_name_
              << "  output=" << output_name_
              << "  output_size=" << output_size_ << "\n";
}

std::vector<float> ONNXInference::run(const std::vector<float>& input) {
    auto memory_info = Ort::MemoryInfo::CreateCpu(
        OrtArenaAllocator, OrtMemTypeDefault);

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info,
        const_cast<float*>(input.data()),
        input.size(),
        input_shape_.data(),
        input_shape_.size());

    const char* input_name_ref  = input_name_.c_str();
    const char* output_name_ref = output_name_.c_str();

    auto output_tensors = session_.Run(
        Ort::RunOptions{nullptr},
        &input_name_ref,  &input_tensor, 1,
        &output_name_ref, 1);

    const float* output_data =
        output_tensors[0].GetTensorData<float>();
    return std::vector<float>(output_data, output_data + output_size_);
}

std::string ONNXInference::get_input_name()  const { return input_name_;  }
std::string ONNXInference::get_output_name() const { return output_name_; }
std::size_t ONNXInference::get_output_size() const { return output_size_;  }

#endif // ONNX_STUB
