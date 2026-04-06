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

if(ONNX_STUB)
  add_compile_definitions(ONNX_STUB)
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
