#pragma once

#include <cstdint>

#include "PlatformSymbols.h"

// increment this when making incompatible changes to the plugin ABI
#define PLUGIN_API_VERSION 1

using PluginDecoder = void*;

enum PluginCapabilities : uint32_t {
    PLUGIN_CAPABILITY_STATIC    = 1 << 0,
    PLUGIN_CAPABILITY_ANIMATION = 1 << 1
};

struct PluginImageData {
    uint8_t* pixels = nullptr;

    int width = 0;
    int height = 0;
    int channels = 0;
};

struct PluginImageFrame {
    uint8_t* pixels;

    int width;
    int height;
    int channels;

    uint32_t durationMs;
};

struct PluginImageSequence {
    PluginImageFrame* frames;
    int frameCount;

    bool animated;
};

using PluginCreateFunc = PluginDecoder (*)();
using PluginDestroyFunc = void (*)(PluginDecoder);

using PluginSupportsExtensionFunc = bool (*)(PluginDecoder, const char*);

using PluginDecodeImageFunc = PluginImageData (*)(PluginDecoder, const char*);
using PluginFreeImageDataFunc = void (*)(PluginDecoder, PluginImageData*);

using PluginDecodeAnimationFunc = PluginImageSequence (*)(PluginDecoder, const char*);
using PluginFreeAnimationFunc = void (*)(PluginDecoder, PluginImageSequence*);

struct PluginAPI {
    uint32_t apiVersion;

    const char* name;
    const char* version;
    const char* displayName;
    const char* developerName;

    uint32_t capabilities;

    int priority;

    PluginCreateFunc create;
    PluginDestroyFunc destroy;

    PluginSupportsExtensionFunc supportsExtension;

    PluginDecodeImageFunc decodeImage;
    PluginFreeImageDataFunc freeImageData;

    PluginDecodeAnimationFunc decodeAnimation;
    PluginFreeAnimationFunc freeAnimation;
};

using GetPluginAPIFunc = PluginAPI (*)();