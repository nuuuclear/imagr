#include "PluginAPI.h"

#define QOI_IMPLEMENTATION
#include "qoi.h"

#include <algorithm>
#include <cctype>
#include <string>

class QoiDecoder {
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

        return ext == ".qoi";
    }

    PluginImageData decodeImage(const std::string& filePath) {
        PluginImageData result{};

        qoi_desc desc{};

        void* pixels = qoi_read(
            filePath.c_str(),
            &desc,
            4 //rgba
        );

        if (!pixels) return {};

        result.pixels = static_cast<uint8_t*>(pixels);
        result.width = static_cast<int>(desc.width);
        result.height = static_cast<int>(desc.height);
        result.channels = 4;

        return result;
    }

    void freeImageData(PluginImageData& data) {
        if (data.pixels) {
            qoi_free_pixels(data.pixels);
        }

        data = {};
    }
};

static PluginDecoder Create() {
    return new QoiDecoder();
}

static void Destroy(PluginDecoder decoder) {
    delete static_cast<QoiDecoder*>(decoder);
}

static bool SupportsExtension(PluginDecoder decoder, const char* extension) {
    if (!decoder || !extension) {
        return false;
    }

    return static_cast<QoiDecoder*>(decoder)->supportsExtension(extension);
}

static PluginImageData DecodeImage(PluginDecoder decoder, const char* filePath) {
    if (!decoder || !filePath) {
        return {};
    }

    return static_cast<QoiDecoder*>(decoder)->decodeImage(filePath);
}

static void FreeImageData(PluginDecoder decoder, PluginImageData* imageData) {
    if (!decoder || !imageData) {
        return;
    }

    static_cast<QoiDecoder*>(decoder)->freeImageData(*imageData);
}

PLUGIN_EXPORT PluginAPI GetPluginAPI() {
    return PluginAPI{
        PLUGIN_API_VERSION,
        "qoi",
        "1.0.0",
        "QOI image support",
        "Internal",
        50,

        Create,
        Destroy,
        SupportsExtension,
        DecodeImage,
        FreeImageData
    };
}