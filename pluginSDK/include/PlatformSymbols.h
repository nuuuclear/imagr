#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>

    using PluginHandle = HMODULE;

    #define PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
    #define PLATFORM_POSIX
    
    #include <dlfcn.h>

    using PluginHandle = void*;
    
    #define PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#endif

