cmake_minimum_required(VERSION 3.16)

# --- Library: onnx_inference (stub) ---
add_library(onnx_inference
    onnx_inference.cpp
)

target_include_directories(onnx_inference PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# --- Library: vision_encoder ---
add_library(vision_encoder
    vision_encoder/vision_encoder.cpp
)

target_include_directories(vision_encoder PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vision_encoder
)

target_link_libraries(vision_encoder PUBLIC
    onnx_inference
)

# --- Standalone smoke-test binary ---
add_executable(inference_engine_test
    main_test.cpp
)

target_link_libraries(inference_engine_test PRIVATE
    vision_encoder
)
