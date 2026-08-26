#include "PluginAPI.h"
#include "PluginStaticImage.h"

#include <algorithm>
#include <cctype>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// stb_image plugin, this only supports bitmap, png and jpeg.

class StbDecoder {
public:
    bool supportsExtension(const std::string& extension) {
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

        return ext == ".png" 
            || ext == ".jpg" 
            || ext == ".jpeg" 
            || ext == ".bmp";
    }

    PluginImageData decodeImage(const std::string& filePath) {
        PluginImageData data{};

        int channels = 0;

        data.pixels = stbi_load(
            filePath.c_str(),
            &data.width,
            &data.height,
            &channels,
            4
        );

        if (!data.pixels) return {};

        data.channels = 4;

        return data;
    }

    void freeImageData(PluginImageData& data) {
        if (data.pixels) {
            stbi_image_free(data.pixels);
        }

        data = {};
    }
};

static PluginDecoder Create() {
    return new StbDecoder();
}

static void Destroy(PluginDecoder decoder) {
    delete static_cast<StbDecoder*>(decoder);
}

static bool SupportsExtension(PluginDecoder decoder, const char* extension) {
    if (!decoder || !extension)return false;

    return static_cast<StbDecoder*>(decoder)->supportsExtension(extension);
}

static PluginImageData DecodeImage(PluginDecoder decoder, const char* filePath) {
    if (!decoder || !filePath) return {};

    return static_cast<StbDecoder*>(decoder)->decodeImage(filePath);
}

static void FreeImageData(PluginDecoder decoder, PluginImageData* imageData) {
    if (!decoder || !imageData) return;

    static_cast<StbDecoder*>(decoder)->freeImageData(*imageData);
}

PLUGIN_EXPORT PluginAPI GetPluginAPI() {
    return PluginAPI{
        PLUGIN_API_VERSION,
        "stb_image",
        "1.0.0",
        "stb_image general image support",
        "Internal",
        50,

        PLUGIN_CAPABILITY_STATIC,

        Create,
        Destroy,
        SupportsExtension,
        DecodeImage,
        FreeImageData,

        PluginDecodeAnimationUnsupported,
        PluginFreeAnimationUnsupported
    };
}