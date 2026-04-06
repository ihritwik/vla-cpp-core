#pragma once

#include <memory>
#include <string>
#include <vector>
#include <random>

// Forward declaration — avoids pulling onnx_inference.hpp into every TU.
class ONNXInference;

/// @brief Extracts 512-dimensional embedding vectors from preprocessed image
///        tensors. Wraps ONNXInference with a vision-specific output size.
class VisionEncoder {
public:
    /// @brief Constructs a VisionEncoder backed by the given ONNX model.
    /// @param model_path Path to a vision ONNX model (e.g. CLIP ViT-B/32).
    explicit VisionEncoder(const std::string& model_path);

    ~VisionEncoder();

    /// @brief Encodes a preprocessed image tensor into a 512-dim embedding.
    /// @param tensor Flat float tensor, size 224*224*3.
    /// @return 512-dimensional embedding vector.
    std::vector<float> encode(const std::vector<float>& tensor);

    static constexpr std::size_t EMBEDDING_DIM = 512;

private:
    std::unique_ptr<ONNXInference> inference_;
};

// =============================================================================

/// @brief Deterministic dummy encoder for testing — no model required.
///
/// Returns reproducible embeddings in [-1, 1] by seeding std::mt19937 with
/// a checksum derived from the input tensor.  Same input → same output.
class DummyEncoder {
public:
    DummyEncoder() = default;

    /// @brief Returns a 512-dim embedding deterministic in the input content.
    /// @param tensor Flat float input tensor (any size).
    /// @return 512-dimensional embedding in [-1.0f, 1.0f].
    std::vector<float> encode(const std::vector<float>& tensor);

    static constexpr std::size_t EMBEDDING_DIM = 512;
};
