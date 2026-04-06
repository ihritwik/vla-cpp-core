#include "../vendor/catch2.hpp"
#include "../../inference_engine/onnx_inference.hpp"
#include <cmath>
#include <algorithm>

TEST_CASE("Stub returns correct size", "[onnx]") {
    ONNXInference onnx("dummy.onnx");
    std::vector<float> input(224 * 224 * 3, 0.5f);
    auto out = onnx.run(input, 512);
    REQUIRE(out.size() == 512u);
}

TEST_CASE("Stub no throw", "[onnx]") {
    ONNXInference onnx("dummy.onnx");
    std::vector<float> input(10, 1.0f);
    REQUIRE_NOTHROW(onnx.run(input, 128));
}

TEST_CASE("No NaN or Inf in stub output", "[onnx]") {
    ONNXInference onnx("dummy.onnx");
    std::vector<float> input(100, 0.0f);
    auto out = onnx.run(input, 256);
    REQUIRE(std::all_of(out.begin(), out.end(),
                        [](float v) { return std::isfinite(v); }));
}
