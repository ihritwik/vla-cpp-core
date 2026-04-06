cmake_minimum_required(VERSION 3.16)

# --- Library: action_decoder ---
add_library(action_decoder
    action_decoder.cpp
    serial_publisher/serial_publisher.cpp
)

target_include_directories(action_decoder PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/serial_publisher
)

# --- Binary: cmd_reader ---
add_executable(cmd_reader
    serial_publisher/cmd_reader.cpp
)

# --- Standalone smoke-test binary ---
add_executable(action_decoder_test
    main_test.cpp
)

target_link_libraries(action_decoder_test PRIVATE
    action_decoder
)
