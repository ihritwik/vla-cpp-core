cmake_minimum_required(VERSION 3.16)

find_package(OpenCV 4 REQUIRED)

add_executable(benchmark_pipeline
    benchmark_pipeline.cpp
)

target_include_directories(benchmark_pipeline PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(benchmark_pipeline PRIVATE
    camera_capture
    onnx_inference
    vision_encoder
    action_decoder
    ${OpenCV_LIBS}
)
