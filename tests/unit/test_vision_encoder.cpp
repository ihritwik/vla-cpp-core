#include "../vendor/catch2.hpp"
#include "../../inference_engine/vision_encoder/vision_encoder.hpp"
#include <algorithm>
#include <cmath>

TEST_CASE("DummyEncoder output dimension", "[vision_encoder]") {
    DummyEncoder enc;
    std::vector<float> input(224 * 224 * 3, 0.5f);
    auto emb = enc.encode(input);
    REQUIRE(emb.size() == 512u);
}

TEST_CASE("DummyEncoder values in [-1, 1]", "[vision_encoder]") {
    DummyEncoder enc;
    std::vector<float> input(224 * 224 * 3, 0.3f);
    auto emb = enc.encode(input);
    REQUIRE(std::all_of(emb.begin(), emb.end(),
                        [](float v) { return v >= -1.0f && v <= 1.0f; }));
}

TEST_CASE("DummyEncoder determinism", "[vision_encoder]") {
    DummyEncoder enc;
    std::vector<float> input(512, 0.42f);
    auto emb1 = enc.encode(input);
    auto emb2 = enc.encode(input);
    REQUIRE(emb1 == emb2);
}
