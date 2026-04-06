#include "camera_capture.hpp"

CameraCapture::CameraCapture(int device_index)
    : device_index_(device_index) {}

bool CameraCapture::open() {
    cap_.open(device_index_);
    return cap_.isOpened();
}

std::vector<float> CameraCapture::capture_and_preprocess() {
    if (!cap_.isOpened()) {
        return {};
    }
    cv::Mat frame;
    if (!cap_.read(frame) || frame.empty()) {
        return {};
    }
    return preprocess(frame);
}

std::vector<float> CameraCapture::preprocess(const cv::Mat& frame) {
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(TARGET_WIDTH, TARGET_HEIGHT));

    cv::Mat float_mat;
    resized.convertTo(float_mat, CV_32F, 1.0f / 255.0f);

    // float_mat is HxWxC (224x224x3) in BGR order — flatten to vector.
    // ptr<float>() is the idiomatic OpenCV way to get a typed pointer.
    const float* begin = float_mat.ptr<float>();
    const float* end   = begin + TARGET_WIDTH * TARGET_HEIGHT * 3;
    return std::vector<float>(begin, end);
}

void CameraCapture::release() {
    cap_.release();
}

bool CameraCapture::is_open() const {
    return cap_.isOpened();
}
