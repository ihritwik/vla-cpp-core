cmake_minimum_required(VERSION 3.16)

# ── ONNX Runtime linkage (real mode only) ────────────────────────────────────
if(NOT ONNX_STUB)
    find_library(ONNXRUNTIME_LIB
        NAMES onnxruntime
        PATHS /usr/local/lib
        REQUIRED)
    message(STATUS "[inference_engine] ONNX Runtime found: ${ONNXRUNTIME_LIB}")
else()
    message(STATUS "[inference_engine] Stub mode — ONNX Runtime not required")
endif()

# ── Library: onnx_inference ───────────────────────────────────────────────────
add_library(onnx_inference
    onnx_inference.cpp
)

target_include_directories(onnx_inference PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(NOT ONNX_STUB)
    target_include_directories(onnx_inference PRIVATE /usr/local/include)
    target_link_libraries(onnx_inference PUBLIC ${ONNXRUNTIME_LIB})
endif()

# ── Library: vision_encoder ───────────────────────────────────────────────────
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

# ── Standalone smoke-test binary ──────────────────────────────────────────────
add_executable(inference_engine_test
    main_test.cpp
)

target_link_libraries(inference_engine_test PRIVATE
    vision_encoder
)
