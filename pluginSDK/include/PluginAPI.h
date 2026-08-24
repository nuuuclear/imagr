#pragma once

#include <cstdint>

#include "PlatformSymbols.h"

// increment this when making incompatible changes to the plugin ABI
#define PLUGIN_API_VERSION 1

using PluginDecoder = void*;

struct PluginImageData {
    uint8_t* pixels = nullptr;

    int width = 0;
    int height = 0;
    int channels = 0;
};

struct PluginAPI {
    uint32_t apiVersion;

    const char* name;
    const char* version;
    const char* displayName;
    const char* developerName;

    // high priority plugins get used when multiple plugins support the same file extensions
    int priority;

    PluginDecoder (*create)();
    void (*destroy)(PluginDecoder decoder);

    // returns true if this decoder supports the suplied extension
    // example: ".png", ".jpg", or ".webp"
    bool (*supportsExtension)(
        PluginDecoder decoder,
        const char* extension
    );

    PluginImageData (*decodeImage)(
        PluginDecoder decoder,
        const char* filePath
    );

    void (*freeImageData)(
        PluginDecoder decoder,
        PluginImageData* imageData
    );
};

using GetPluginAPIFunc = PluginAPI (*)();