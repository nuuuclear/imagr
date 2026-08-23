#include "IImageDecoder.h"
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" 

// stb_image plugin, this only supports bitmap, png and jpeg.

class StbDecoder : public IImageDecoder {
public:
    bool supportsExtension(const std::string& ext) override {
        std::string lowerExt = ext;
        std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
        return (lowerExt == ".png" || lowerExt == ".jpg" || lowerExt == ".jpeg" || lowerExt == ".bmp");
    }

    RawImageData decodeImage(const std::string& filePath) override {
        RawImageData data;
        data.pixels = stbi_load(filePath.c_str(), &data.width, &data.height, &data.channels, 4);
        data.channels = 4; 
        return data;
    }

    void freeImageData(RawImageData& data) override {
        if (data.pixels) {
            stbi_image_free(data.pixels);
            data.pixels = nullptr;
        }
    }
};

CV_EXPORT IImageDecoder* createDecoder() {
    return new StbDecoder();
}

CV_EXPORT void destroyDecoder(IImageDecoder* decoder) {
    delete decoder;
}
