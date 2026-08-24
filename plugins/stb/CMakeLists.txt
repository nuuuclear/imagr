cmake_minimum_required(VERSION 3.10)

project(imagr_stb_plugin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(imagr_stb SHARED
    stbDecoder.cpp
)

target_link_libraries(imagr_stb PRIVATE
    imagr_plugin_sdk
    stbimage
)

set_target_properties(imagr_stb PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_BINARY_DIR}/plugins"
)