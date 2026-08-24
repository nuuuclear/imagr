#pragma once

#include <string>
#include <cstdint>

struct RawImageData {
    uint8_t* pixels = nullptr;

    int width = 0;
    int height = 0;
    int channels = 0;
};

class IImageDecoder {
public:
    virtual ~IImageDecoder() = default;

    virtual bool supportsExtension(
        const std::string& extension
    ) = 0;

    virtual RawImageData decodeImage(
        const std::string& filePath
    ) = 0;

    virtual void freeImageData(
        RawImageData& data
    ) = 0;
};