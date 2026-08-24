#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>

    using PluginHandle = HMODULE;

#else
    #define PLATFORM_POSIX

    #include <dlfcn.h>

    using PluginHandle = void*;
#endif

#include <iostream>
#include <string>

inline PluginHandle LoadPluginLibrary(
    const std::wstring& path
) {
#if defined(PLATFORM_WINDOWS)
    HMODULE handle = LoadLibraryW(path.c_str());

    if (!handle) {
        DWORD error = GetLastError();

        std::wcerr
            << L"[Plugin Error] Failed to load plugin: "
            << path
            << L" (error "
            << error
            << L")\n";
    }

    return handle;

#else
    std::string stringPath(
        path.begin(),
        path.end()
    );

    PluginHandle handle =
        dlopen(
            stringPath.c_str(),
            RTLD_LAZY
        );

    if (!handle) {
        std::cerr
            << "[Plugin Error] Failed to load plugin: "
            << dlerror()
            << "\n";
    }

    return handle;

#endif
}

inline void* GetPluginSymbol(PluginHandle handle, const std::string& symbol) {
    if (!handle)
        return nullptr;

#if defined(PLATFORM_WINDOWS)
    FARPROC proc = GetProcAddress(
        handle,
        symbol.c_str()
    );

    if (!proc) {
        DWORD error = GetLastError();

        std::cerr
            << "<Plugins> ERROR: Could not find symbol '"
            << symbol
            << "' (error "
            << error
            << ")\n";

        return nullptr;
    }

    return reinterpret_cast<void*>(proc);
#else
    dlerror();

    void* proc = dlsym(
        handle,
        symbol.c_str()
    );

    const char* error = dlerror();

    if (error) {
        std::cerr
            << "<Plugins> ERROR: Could not find symbol '"
            << symbol
            << "': "
            << error
            << "\n";

        return nullptr;
    }

    return proc;

#endif
}

inline void FreePluginLibrary(PluginHandle handle) {
    if (!handle)
        return;

#if defined(PLATFORM_WINDOWS)
    FreeLibrary(handle);

#else
    dlclose(handle);

#endif
}