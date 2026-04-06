// Standalone smoke-test for ONNXInference and VisionEncoder.
// Compile with: see inference_engine/CMakeLists.txt
// Run:          ./inference_engine_test

#include "onnx_inference.hpp"
#include "vision_encoder/vision_encoder.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>

int main() {
    // --- ONNXInference stub ---
    ONNXInference onnx("dummy_model.onnx");
    std::vector<float> input(224 * 224 * 3, 0.5f);
    auto out = onnx.run(input, 512);
    assert(out.size() == 512);
    assert(std::all_of(out.begin(), out.end(),
                       [](float v) { return std::isfinite(v); }));
    std::cout << "[PASS] ONNXInference stub returns 512 finite values.\n";

    // --- DummyEncoder ---
    DummyEncoder enc;
    auto emb1 = enc.encode(input);
    auto emb2 = enc.encode(input);
    assert(emb1.size() == 512);
    assert(emb1 == emb2);  // deterministic
    assert(std::all_of(emb1.begin(), emb1.end(),
                       [](float v) { return v >= -1.0f && v <= 1.0f; }));
    std::cout << "[PASS] DummyEncoder returns 512-dim deterministic embedding.\n";

    std::cout << "All inference_engine tests passed.\n";
    return 0;
}
