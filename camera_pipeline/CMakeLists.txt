cmake_minimum_required(VERSION 3.16)

find_package(OpenCV 4 REQUIRED)

# --- Library: camera_capture ---
add_library(camera_capture
    camera_capture.cpp
)

target_include_directories(camera_capture PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(camera_capture PUBLIC
    ${OpenCV_LIBS}
)

# --- Standalone smoke-test binary ---
add_executable(camera_pipeline_test
    main_test.cpp
)

target_link_libraries(camera_pipeline_test PRIVATE
    camera_capture
)

# ring_buffer is header-only; no library target needed.
# ring_buffer_test.cpp is a standalone file compiled separately if desired.
