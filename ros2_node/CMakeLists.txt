cmake_minimum_required(VERSION 3.22)
project(vla_inference_node)

# ── C++17 ─────────────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── ONNX Runtime: auto-detect; fall back to stub if not found ────────────────
find_library(ONNXRUNTIME_LIB
    NAMES onnxruntime
    PATHS /usr/local/lib)

if(ONNXRUNTIME_LIB)
    message(STATUS "[vla_inference_node] ONNX Runtime found: ${ONNXRUNTIME_LIB} — real inference enabled")
else()
    message(STATUS "[vla_inference_node] ONNX Runtime not found — stub mode active")
    add_compile_definitions(ONNX_STUB)
endif()

# ── ROS2 / ament dependencies ─────────────────────────────────────────────────
find_package(ament_cmake REQUIRED)
find_package(rclcpp      REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs    REQUIRED)
find_package(cv_bridge   REQUIRED)
find_package(OpenCV      REQUIRED)

# ── Phase 1 source files (included directly, no install step needed) ──────────
set(PHASE1_SOURCES
    ../camera_pipeline/camera_capture.cpp
    ../inference_engine/onnx_inference.cpp
    ../inference_engine/vision_encoder/vision_encoder.cpp
    ../action_decoder/action_decoder.cpp
    ../action_decoder/serial_publisher/serial_publisher.cpp
)

set(PHASE1_INCLUDE_DIRS
    ../camera_pipeline
    ../inference_engine
    ../action_decoder
    ../action_decoder/serial_publisher
)

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(vla_node
    src/main.cpp
    src/vla_node.cpp
    ${PHASE1_SOURCES}
)

target_include_directories(vla_node
    PRIVATE
        include
        ${PHASE1_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
)

ament_target_dependencies(vla_node
    rclcpp
    sensor_msgs
    std_msgs
    cv_bridge
)

target_link_libraries(vla_node
    ${OpenCV_LIBS}
)

if(ONNXRUNTIME_LIB)
    target_include_directories(vla_node PRIVATE /usr/local/include)
    target_link_libraries(vla_node ${ONNXRUNTIME_LIB})
endif()

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS vla_node
    DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
    DESTINATION include
)

ament_package()
