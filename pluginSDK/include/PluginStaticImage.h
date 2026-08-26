#pragma once

#include "PluginAPI.h"

inline PluginImageSequence PluginDecodeAnimationUnsupported(PluginDecoder, const char*) {
    return {};
}

inline void PluginFreeAnimationUnsupported(PluginDecoder, PluginImageSequence* animation) {
    if (animation) {
        *animation = {};
    }
}