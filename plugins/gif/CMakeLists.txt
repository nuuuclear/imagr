cmake_minimum_required(VERSION 3.10)

project(imagr_gif_plugin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

add_library(giflib STATIC
    external/giflib/dgif_lib.c
    external/giflib/egif_lib.c
    external/giflib/gif_err.c
    external/giflib/gif_font.c
    external/giflib/gifalloc.c
    external/giflib/openbsd-reallocarray.c
)
target_include_directories(giflib PUBLIC external/giflib/)

add_library(imagr_gif SHARED
    gifDecoder.cpp
)

target_link_libraries(imagr_gif PRIVATE
    imagr_plugin_sdk
    giflib
)

set_target_properties(imagr_gif PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_BINARY_DIR}/plugins"
)