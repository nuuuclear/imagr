#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
    #include <windows.h>
    using PluginHandle = HMODULE;
    #define CV_EXPORT extern "C" __declspec(dllexport)
#else
    #define PLATFORM_POSIX
    #include <dlfcn.h>
    using PluginHandle = void*;
    #define CV_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#include <string>
#include <iostream>

inline PluginHandle LoadPluginLibrary(const std::wstring& path) {
#if defined(PLATFORM_WINDOWS)
    return LoadLibraryW(path.c_str());
#else
    // convert wstring back to string for POSIX dlopen
    std::string sPath(path.begin(), path.end());
    // RTLD_LAZY binds symbols as they get executed
    PluginHandle handle = dlopen(sPath.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "[Plugin Error] dlopen failed: " << dlerror() << "\n";
    }
    return handle;
#endif
}

inline void* getPluginSymbol(PluginHandle handle, const std::string& symbol) {
#if defined(PLATFORM_WINDOWS)
    return (void*)GetProcAddress(handle, symbol.c_str());
#else
    return dlsym(handle, symbol.c_str());
#endif
}

inline void freePluginLibrary(PluginHandle handle) {
    if (!handle) return;
#if defined(PLATFORM_WINDOWS)
    FreeLibrary(handle);
#else
    dlclose(handle);
#endif
}
