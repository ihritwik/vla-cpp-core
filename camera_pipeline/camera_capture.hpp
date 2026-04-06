#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

/// @brief Captures frames from a camera or synthetic source and preprocesses
///        them into normalized float tensors for VLA inference.
class CameraCapture {
public:
    /// @brief Constructs a CameraCapture bound to a specific device.
    /// @param device_index OpenCV device index (0 = default webcam).
    explicit CameraCapture(int device_index);

    /// @brief Opens the video capture device.
    /// @return true if the device was successfully opened, false otherwise.
    bool open();

    /// @brief Captures one frame and preprocesses it into a float tensor.
    ///        Resizes to 224x224, converts BGR to float, normalizes to [0,1].
    /// @return Flat float vector of size 224*224*3 (HWC order), or empty on
    ///         failure.
    std::vector<float> capture_and_preprocess();

    /// @brief Preprocesses an existing cv::Mat into a float tensor.
    ///        Useful for testing with synthetic frames.
    /// @param frame Input BGR frame (any size).
    /// @return Flat float vector of size 224*224*3 (HWC order).
    std::vector<float> preprocess(const cv::Mat& frame);

    /// @brief Releases the underlying video capture device.
    void release();

    /// @brief Returns true if the capture device is currently open.
    /// @return Capture open state.
    bool is_open() const;

private:
    int device_index_;
    cv::VideoCapture cap_;

    static constexpr int TARGET_WIDTH  = 224;
    static constexpr int TARGET_HEIGHT = 224;
};
