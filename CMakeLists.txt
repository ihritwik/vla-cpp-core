cmake_minimum_required(VERSION 3.16)
project(vla-cpp-core VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(OpenCV 4 REQUIRED)

option(BUILD_TESTS "Build test suite" ON)
option(BUILD_BENCHMARKS "Build benchmarks" ON)
option(BUILD_ROS2_NODE "Build ROS2 node" OFF)
option(ONNX_STUB "Use stub ONNX mode" ON)
option(ENABLE_ORT_PROFILING
  "Write per-op Chrome-trace JSON via ORT built-in profiler (real mode only)" OFF)

if(ONNX_STUB)
  add_compile_definitions(ONNX_STUB)
endif()

if(ENABLE_ORT_PROFILING AND NOT ONNX_STUB)
  add_compile_definitions(ENABLE_ORT_PROFILING)
  message(STATUS "ORT profiling enabled — trace written to ort_profile_*.json")
elseif(ENABLE_ORT_PROFILING AND ONNX_STUB)
  message(WARNING "ENABLE_ORT_PROFILING has no effect in stub mode (ONNX_STUB=ON)")
endif()

add_subdirectory(camera_pipeline)
add_subdirectory(inference_engine)
add_subdirectory(action_decoder)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()
