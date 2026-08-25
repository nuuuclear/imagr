cmake_minimum_required(VERSION 3.10)
project(imagr)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

set(SDL3_DIR "${CMAKE_SOURCE_DIR}/external/SDL3/cmake/")
set(SDL3_ttf_DIR "${CMAKE_SOURCE_DIR}/external/SDL3_ttf/cmake/")

# externals

find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)

add_library(inicpp OBJECT external/inicpp/ini.cpp)
target_include_directories(inicpp PUBLIC external/inicpp)

# Plugins

add_subdirectory(pluginSDK)

add_subdirectory(plugins/stb)
add_subdirectory(plugins/qoi)
add_subdirectory(plugins/gif)

# imagr executable

add_executable(imagr
    "src/Main.cpp"
    "src/App.cpp"
    "src/App.h"
    "src/Viewer.cpp"
    "src/Viewer.h"
    "src/PluginManager.h"
    "src/PlatformSymbols.h"
    "src/Paths.h"
    "src/FileSystem.cpp"
    "src/FileSystem.h"
)

target_link_libraries(imagr PRIVATE 
    SDL3::SDL3 
    SDL3_ttf::SDL3_ttf 
    inicpp 
    imagr_plugin_sdk
)

set_target_properties(imagr PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if(UNIX AND NOT APPLE)
    # Linux/BSD: Link to pthread and lm
    target_link_libraries(imagr PRIVATE pthread m)
    # Link to -latomic if needed (for specific compilers/architectures)
    # target_link_libraries(MyApp PRIVATE atomic)
endif()


# copy resources

set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")

add_custom_command(TARGET imagr POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${RESOURCE_DIR}"
    "$<TARGET_FILE_DIR:imagr>/resources"
    COMMENT "Copying resources to build directory"
)


# install rules

install(TARGETS imagr RUNTIME DESTINATION .)

# copy libraries
# probs wont work properly on linux, sdl may need to built seperatly there.

if(WIN32)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_ttf/x86_64-w64-mingw32/bin/SDL3_ttf.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/SDL3_ttf.dll"
        COPYONLY
    )

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3/x86_64-w64-mingw32/bin/SDL3.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/SDL3.dll"
        COPYONLY
    )

    install(FILES $<TARGET_FILE:SDL3::SDL3> 
        DESTINATION .
    )

    install(FILES $<TARGET_FILE:SDL3_ttf::SDL3_ttf> 
        DESTINATION .
    )
endif()

install(DIRECTORY resources/
    DESTINATION resources
    PATTERN "*.log" EXCLUDE
    PATTERN ".DS_Store" EXCLUDE
)