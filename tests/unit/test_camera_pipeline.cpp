#include "../vendor/catch2.hpp"
#include "../../camera_pipeline/camera_capture.hpp"
#include <algorithm>
#include <opencv2/opencv.hpp>

TEST_CASE("Preprocess black frame", "[camera_pipeline]") {
    CameraCapture cap(0);
    cv::Mat black(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
    auto result = cap.preprocess(black);

    REQUIRE(result.size() == 224u * 224u * 3u);
    REQUIRE(std::all_of(result.begin(), result.end(),
                        [](float v) { return v >= 0.0f; }));
    REQUIRE(std::all_of(result.begin(), result.end(),
                        [](float v) { return v <= 1.0f; }));
}

TEST_CASE("Preprocess white frame", "[camera_pipeline]") {
    CameraCapture cap(0);
    cv::Mat white(480, 640, CV_8UC3, cv::Scalar(255, 255, 255));
    auto result = cap.preprocess(white);

    REQUIRE(result.size() == 224u * 224u * 3u);
    for (float v : result) {
        REQUIRE(v == Approx(1.0f).epsilon(0.01));
    }
}
