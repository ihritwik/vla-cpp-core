#pragma once

#include <string>
#include <vector>

// =============================================================================
// Compile-time switch:
//   ONNX_STUB defined  → zero-tensor stub, no external dependencies (CI mode)
//   ONNX_STUB not defined → real ONNX Runtime C++ API (RPi / device build)
// =============================================================================

#ifndef ONNX_STUB
#include <memory>
#include <onnxruntime_cxx_api.h>
#endif

/// @brief Model-agnostic ONNX inference wrapper.
///
/// Stub mode (ONNX_STUB): returns zero-filled tensors of the requested size
/// without loading any model file — no ONNX Runtime dependency required.
///
/// Real mode: loads the model with ORT CPU EP, 4 intra-op threads, and full
/// graph optimization.  Input shape is fixed to [1, 3, 224, 224] (CLIP).
class ONNXInference {
public:

#ifdef ONNX_STUB
    // ── Stub API ──────────────────────────────────────────────────────────────

    /// @brief Constructs the stub wrapper (model is not loaded).
    /// @param model_path Logged but otherwise unused in stub mode.
    explicit ONNXInference(const std::string& model_path);

    /// @brief Returns a zero-filled output tensor of the requested size.
    /// @param input       Input tensor (ignored in stub mode).
    /// @param output_size Number of float elements to return.
    /// @return Zero-filled vector of length output_size.
    std::vector<float> run(const std::vector<float>& input,
                           std::size_t               output_size);

    /// @brief Returns the stub input name "input".
    std::string get_input_name()  const;

    /// @brief Returns the stub output name "output".
    std::string get_output_name() const;

private:
    std::string model_path_;

#else
    // ── Real ONNX Runtime API ─────────────────────────────────────────────────

    /// @brief Loads the model and queries input/output tensor names.
    /// @param model_path  Path to the .onnx file.
    /// @param output_size Expected number of output elements (e.g. 768 for CLIP).
    ONNXInference(const std::string& model_path, std::size_t output_size);

    /// @brief Runs a forward pass through the loaded model.
    /// @param input Flat float32 tensor matching input_shape_ (3*224*224 = 150528).
    /// @return Flat float32 output vector of length output_size_.
    std::vector<float> run(const std::vector<float>& input);

    /// @brief Returns the first input node name as queried from the model.
    std::string get_input_name()  const;

    /// @brief Returns the first output node name as queried from the model.
    std::string get_output_name() const;

    /// @brief Returns the expected output tensor size baked into this instance.
    std::size_t get_output_size() const;

private:
    Ort::Env                        env_;
    Ort::SessionOptions             session_options_;
    Ort::Session                    session_;
    Ort::AllocatorWithDefaultOptions allocator_;

    std::string              input_name_;
    std::string              output_name_;
    std::size_t              output_size_;
    std::vector<int64_t>     input_shape_;   ///< [1, 3, 224, 224]
#endif
};
