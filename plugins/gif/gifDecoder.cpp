#include "PluginAPI.h"

#include <gif_lib.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>

// TODO: animation!!!

class GifDecoder {
public:
    bool supportsExtension(const std::string& extension){
        std::string ext = extension;

        std::transform(
            ext.begin(),
            ext.end(),
            ext.begin(),
            [](unsigned char c) {
                return static_cast<char>(
                    std::tolower(c)
                );
            }
        );

        return ext == ".gif";
    }

    PluginImageData decodeImage(const std::string& filePath) {
        PluginImageData result{};

        int error = 0;

        GifFileType* gif = DGifOpenFileName(
            filePath.c_str(),
            &error
        );

        if (!gif) return {};

        if (DGifSlurp(gif) != GIF_OK) {
            DGifCloseFile(gif, &error);
            return {};
        }

        if (gif->ImageCount <= 0) {
            DGifCloseFile(gif, &error);
            return {};
        }

        const SavedImage& frame = gif->SavedImages[0];

        const int width = gif->SWidth;
        const int height = gif->SHeight;

        const ColorMapObject* colorMap = frame.ImageDesc.ColorMap ? frame.ImageDesc.ColorMap : gif->SColorMap;

        if (!colorMap) {
            DGifCloseFile(gif, &error);
            return {};
        }

        size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);

        size_t bufferSize = pixelCount * 4;

        auto* pixels = new uint8_t[bufferSize];

        std::memset(pixels, 0, bufferSize);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                size_t index = static_cast<size_t>(y) * width + x;
                uint8_t colorIndex = frame.RasterBits[index];

                const GifColorType& color = colorMap->Colors[colorIndex];

                pixels[index * 4 + 0] = color.Red;
                pixels[index * 4 + 1] = color.Green;
                pixels[index * 4 + 2] = color.Blue;
                pixels[index * 4 + 3] = 255;
            }
        }

        result.pixels = pixels;
        result.width = width;
        result.height = height;
        result.channels = 4;

        DGifCloseFile(gif, &error);

        return result;
    }

    void freeImageData(PluginImageData& data) {
        delete[] data.pixels;

        data = {};
    }
};

static PluginDecoder Create() {
    return new GifDecoder();
}

static void Destroy(PluginDecoder decoder) {
    delete static_cast<GifDecoder*>(decoder);
}

static bool SupportsExtension(PluginDecoder decoder, const char* extension) {
    if (!decoder || !extension) return false;

    return static_cast<GifDecoder*>(decoder)->supportsExtension(extension);
}

static PluginImageData DecodeImage(PluginDecoder decoder, const char* filePath) {
    if (!decoder || !filePath) return {};

    return static_cast<GifDecoder*>(decoder)->decodeImage(filePath);
}

static void FreeImageData(PluginDecoder decoder, PluginImageData* imageData) {
    if (!decoder || !imageData) return;

    static_cast<GifDecoder*>(decoder)->freeImageData(*imageData);
}

PLUGIN_EXPORT PluginAPI GetPluginAPI() {
    return PluginAPI{
        PLUGIN_API_VERSION,
        "gif_decoder",
        "1.0.0",
        "GIF support",
        "Internal",
        50,

        Create,
        Destroy,
        SupportsExtension,
        DecodeImage,
        FreeImageData
    };
}