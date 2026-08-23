#pragma once
#include <string>
#include "PlatformSymbols.h"

struct RawImageData {
    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;
};

class IImageDecoder {
public:
    virtual ~IImageDecoder() = default;
    virtual bool supportsExtension(const std::string& ext) = 0;
    virtual RawImageData decodeImage(const std::string& filePath) = 0;
    virtual void freeImageData(RawImageData& data) = 0;
};

using CreateDecoderFunc = IImageDecoder* (*)();
using DestroyDecoderFunc = void (*)(IImageDecoder*);
