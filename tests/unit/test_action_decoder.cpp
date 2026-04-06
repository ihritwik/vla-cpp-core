#include "../vendor/catch2.hpp"
#include "../../action_decoder/action_decoder.hpp"
#include <algorithm>

static constexpr float PI = 3.14159265358979323846f;

TEST_CASE("Discrete mode valid action index", "[action_decoder]") {
    ActionDecoder dec(Mode::DISCRETE);
    std::vector<float> logits(7, 0.0f);
    auto cmd = dec.decode(logits);
    // All valid ranges
    for (int i = 0; i < 6; ++i) {
        REQUIRE(cmd.joint_angles[i] >= -PI);
        REQUIRE(cmd.joint_angles[i] <=  PI);
    }
    REQUIRE((cmd.gripper_state == 0 || cmd.gripper_state == 1));
    REQUIRE(cmd.velocity_scale >= 0.0f);
    REQUIRE(cmd.velocity_scale <= 1.0f);
}

TEST_CASE("Continuous mode all fields populated", "[action_decoder]") {
    ActionDecoder dec(Mode::CONTINUOUS);
    std::vector<float> logits = {0.1f, -0.3f, 0.5f, 0.0f, 1.2f, -0.7f,
                                  0.9f, 0.75f};
    auto cmd = dec.decode(logits);

    for (int i = 0; i < 6; ++i) {
        REQUIRE(cmd.joint_angles[i] >= -PI);
        REQUIRE(cmd.joint_angles[i] <=  PI);
    }
    REQUIRE((cmd.gripper_state == 0 || cmd.gripper_state == 1));
    REQUIRE(cmd.velocity_scale >= 0.0f);
    REQUIRE(cmd.velocity_scale <= 1.0f);
}

TEST_CASE("Safety clamping on extreme values", "[action_decoder]") {
    ActionDecoder dec(Mode::CONTINUOUS);
    std::vector<float> extreme(8, 1e6f);
    extreme[6] = -1e6f;  // gripper: logit < 0.5 → 0
    extreme[7] = 1e6f;   // velocity: clamped to 1.0
    auto cmd = dec.decode(extreme);

    for (int i = 0; i < 6; ++i) {
        REQUIRE(cmd.joint_angles[i] >= -PI);
        REQUIRE(cmd.joint_angles[i] <=  PI);
    }
    REQUIRE((cmd.gripper_state == 0 || cmd.gripper_state == 1));
    REQUIRE(cmd.velocity_scale >= 0.0f);
    REQUIRE(cmd.velocity_scale <= 1.0f);
}
