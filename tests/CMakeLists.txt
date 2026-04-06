cmake_minimum_required(VERSION 3.16)

find_package(OpenCV 4 REQUIRED)

# Catch2 header — must be downloaded before building.
# curl -L https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp \
#      -o tests/vendor/catch2.hpp
set(CATCH2_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/vendor/catch2.hpp)

# ── Unit tests ─────────────────────────────────────────────────────────────────

# Shared test_main object (defines CATCH_CONFIG_MAIN once)
add_library(test_main OBJECT unit/test_main.cpp)
target_include_directories(test_main PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Helper to register a unit-test executable.
function(add_unit_test target_name sources)
    add_executable(${target_name}
        ${sources}
        $<TARGET_OBJECTS:test_main>
    )
    target_include_directories(${target_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}
        ${OpenCV_INCLUDE_DIRS}
    )
    target_link_libraries(${target_name} PRIVATE
        camera_capture
        onnx_inference
        vision_encoder
        action_decoder
        ${OpenCV_LIBS}
    )
    add_test(NAME ${target_name} COMMAND ${target_name})
endfunction()

add_unit_test(test_camera_pipeline  unit/test_camera_pipeline.cpp)
add_unit_test(test_ring_buffer      unit/test_ring_buffer.cpp)
add_unit_test(test_onnx_wrapper     unit/test_onnx_wrapper.cpp)
add_unit_test(test_vision_encoder   unit/test_vision_encoder.cpp)
add_unit_test(test_action_decoder   unit/test_action_decoder.cpp)
add_unit_test(test_serial_publisher unit/test_serial_publisher.cpp)

# ── Integration test ───────────────────────────────────────────────────────────

add_executable(test_full_pipeline
    integration/test_full_pipeline.cpp
)

set_target_properties(test_full_pipeline PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/integration
)

target_include_directories(test_full_pipeline PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(test_full_pipeline PRIVATE
    camera_capture
    onnx_inference
    vision_encoder
    action_decoder
    ${OpenCV_LIBS}
)

add_test(NAME integration_full_pipeline
         COMMAND $<TARGET_FILE:test_full_pipeline>)
