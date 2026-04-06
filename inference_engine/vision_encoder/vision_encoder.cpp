#include "vision_encoder.hpp"
#include "../onnx_inference.hpp"
#include <cstring>
#include <memory>

// ---- VisionEncoder ----------------------------------------------------------

#ifdef ONNX_STUB
VisionEncoder::VisionEncoder(const std::string& model_path)
    : inference_(std::make_unique<ONNXInference>(model_path)) {}
#else
VisionEncoder::VisionEncoder(const std::string& model_path)
    : inference_(std::make_unique<ONNXInference>(model_path, EMBEDDING_DIM)) {}
#endif

VisionEncoder::~VisionEncoder() = default;

std::vector<float> VisionEncoder::encode(const std::vector<float>& tensor) {
#ifdef ONNX_STUB
    return inference_->run(tensor, EMBEDDING_DIM);
#else
    return inference_->run(tensor);
#endif
}

// ---- DummyEncoder -----------------------------------------------------------

std::vector<float> DummyEncoder::encode(const std::vector<float>& tensor) {
    // Derive a reproducible seed from the input content so that the same
    // tensor always produces the same embedding.
    static_assert(sizeof(float) == 4, "unexpected float size");
    std::size_t checksum = 0;
    for (float v : tensor) {
        uint32_t raw = 0;
        std::memcpy(&raw, &v, sizeof(raw));
        checksum ^= static_cast<std::size_t>(raw) + 0x9e3779b9u +
                    (checksum << 6) + (checksum >> 2);
    }

    std::mt19937 rng(static_cast<uint32_t>(checksum));
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    std::vector<float> embedding(EMBEDDING_DIM);
    for (float& e : embedding) {
        e = dist(rng);
    }
    return embedding;
}
