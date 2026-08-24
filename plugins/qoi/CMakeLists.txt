cmake_minimum_required(VERSION 3.10)

project(imagr_qoi_plugin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(qoi INTERFACE)
target_include_directories(qoi INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/qoi"
)

add_library(imagr_qoi SHARED
    qoiDecoder.cpp
)

target_link_libraries(imagr_qoi PRIVATE
    imagr_plugin_sdk
    qoi
)

set_target_properties(imagr_qoi PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_BINARY_DIR}/plugins"
)