// Standalone smoke-test for CameraCapture preprocessing.
// Compile with: see camera_pipeline/CMakeLists.txt
// Run:          ./camera_pipeline_test

#include "camera_capture.hpp"
#include <cassert>
#include <iostream>
#include <algorithm>

int main() {
    CameraCapture cap(0);

    // --- Preprocess a synthetic black frame ---
    cv::Mat black(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
    auto result = cap.preprocess(black);

    assert(result.size() == 224u * 224u * 3u);
    assert(std::all_of(result.begin(), result.end(),
                       [](float v) { return v >= 0.0f && v <= 1.0f; }));

    // --- Preprocess a synthetic white frame ---
    cv::Mat white(480, 640, CV_8UC3, cv::Scalar(255, 255, 255));
    auto white_result = cap.preprocess(white);

    assert(white_result.size() == 224u * 224u * 3u);
    for (float v : white_result) {
        assert(v >= 0.99f && v <= 1.001f);
    }

    std::cout << "CameraCapture preprocess tests passed.\n";
    return 0;
}
