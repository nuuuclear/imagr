#include "PluginAPI.h"

#include <gif_lib.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

class GifDecoder {
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

        const int width = gif->SWidth;
        const int height = gif->SHeight;

        if (width <= 0 || height <= 0) {
            DGifCloseFile(gif, &error);
            return {};
        }

        size_t bufferSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;

        std::vector<uint8_t> canvas(bufferSize, 0);

        // render the first frame
        renderFrame(
            gif,
            gif->SavedImages[0],
            canvas
        );

        auto* pixels = new uint8_t[bufferSize];
        std::memcpy(pixels, canvas.data(), bufferSize);

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

    PluginImageSequence decodeAnimation(const std::string& filePath) {
        PluginImageSequence result{};

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

        if (gif->ImageCount <= 0 
        ||  gif->SWidth <= 0 
        ||  gif->SHeight <= 0
        ) {
            DGifCloseFile(gif, &error);
            return {};
        }

        const int width = gif->SWidth;
        const int height = gif->SHeight;

        const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);

        const size_t bufferSize = pixelCount * 4;

        const int frameCount = gif->ImageCount;

        auto* frames = new PluginImageFrame[frameCount]{};

        std::vector<uint8_t> canvas(bufferSize, 0);
        std::vector<uint8_t> previousCanvas;

        int previousDisposal = 0;

        int previousLeft = 0;
        int previousTop = 0;
        int previousWidth = 0;
        int previousHeight = 0;

        for (int i = 0; i < frameCount; ++i) {
            SavedImage& frame = gif->SavedImages[i];

            if (i > 0) {
                if (previousDisposal == 2) {
                    clearRect(
                        canvas,
                        width,
                        height,
                        previousLeft,
                        previousTop,
                        previousWidth,
                        previousHeight
                    );
                } else if (previousDisposal == 3) {
                    if (previousCanvas.size() ==
                        canvas.size()) {

                        canvas = previousCanvas;
                    }
                }
            }

            GifFrameInfo info = getFrameInfo(frame);

            if (info.disposalMethod == 3) {
                previousCanvas = canvas;
            } else {
                previousCanvas.clear();
            }

            renderFrame(gif, frame, canvas, info.transparentColorIndex);

            auto* pixels = new uint8_t[bufferSize];

            std::memcpy(pixels, canvas.data(), bufferSize);

            frames[i].pixels = pixels;
            frames[i].width = width;
            frames[i].height = height;
            frames[i].channels = 4;
            frames[i].durationMs = info.delayMs;

            previousDisposal = info.disposalMethod;
            previousLeft = frame.ImageDesc.Left;
            previousTop = frame.ImageDesc.Top;
            previousWidth = frame.ImageDesc.Width;
            previousHeight = frame.ImageDesc.Height;
        }

        result.frames = frames;
        result.frameCount = frameCount;

        DGifCloseFile(gif, &error);

        return result;
    }

    void freeAnimation(PluginImageSequence& animation) {
        if (animation.frames) {
            for (int i = 0; i < animation.frameCount; ++i) {
                delete[] animation.frames[i].pixels;

                animation.frames[i].pixels = nullptr;
            }
            delete[] animation.frames;
        }
        animation = {};
    }

private:
    struct GifFrameInfo {
        int disposalMethod = 0;
        bool transparent = false;

        int transparentColorIndex = -1;
        uint32_t delayMs = 100;
    };

    GifFrameInfo getFrameInfo(const SavedImage& frame) {
        GifFrameInfo info{};

        // GIF graphics control extension:
        // byte 0: packed fields
        // byte 1: delay low
        // byte 2: delay high
        // byte 3: transparent color index
        for (int i = 0; i < frame.ExtensionBlockCount; ++i) {
            const ExtensionBlock& block = frame.ExtensionBlocks[i];

            if (block.Function != GRAPHICS_EXT_FUNC_CODE) {
                continue;
            }

            if (block.ByteCount < 4 || !block.Bytes) {
                continue;
            }

            const uint8_t packed = static_cast<uint8_t>(
                block.Bytes[0]
            );

            const uint16_t delay = static_cast<uint16_t>(
                static_cast<uint8_t>(
                    block.Bytes[1]
                ) | (
                    static_cast<uint16_t>(
                        static_cast<uint8_t>(
                            block.Bytes[2]
                        )
                    ) << 8
                )
            );

            info.disposalMethod = (packed >> 2) & 0x07;

            info.transparent = (packed & 0x01) != 0;

            info.transparentColorIndex = static_cast<uint8_t>(block.Bytes[3]);

            if (delay == 0) {
                info.delayMs = 100;
            } else {
                info.delayMs = static_cast<uint32_t>(delay) * 10;
            }

            break;
        }

        return info;
    }

    void clearRect(
        std::vector<uint8_t>& canvas,
        int canvasWidth,
        int canvasHeight,
        int left,
        int top,
        int width,
        int height
    ) {
        int startX = std::max(0, left);
        int startY = std::max(0, top);

        int endX = std::min(
            canvasWidth,
            left + width
        );

        int endY = std::min(
            canvasHeight,
            top + height
        );

        for (int y = startY; y < endY; ++y) {
            for (int x = startX; x < endX; ++x) {
                size_t index = (
                    static_cast<size_t>(y) 
                *   static_cast<size_t>(canvasWidth)
                +   static_cast<size_t>(x)
                ) * 4;

                canvas[index + 0] = 0;
                canvas[index + 1] = 0;
                canvas[index + 2] = 0;
                canvas[index + 3] = 0;
            }
        }
    }

    void renderFrame(
        GifFileType* gif,
        const SavedImage& frame,
        std::vector<uint8_t>& canvas,
        int transparentColorIndex = -1
    ) {
        const int canvasWidth = gif->SWidth;
        const int canvasHeight = gif->SHeight;

        const int left = frame.ImageDesc.Left;
        const int top = frame.ImageDesc.Top;

        const int width = frame.ImageDesc.Width;
        const int height = frame.ImageDesc.Height;

        if (!frame.RasterBits 
        ||  width <= 0 
        ||  height <= 0
        ) {
            return;
        }

        const ColorMapObject* colorMap 
            = frame.ImageDesc.ColorMap
            ? frame.ImageDesc.ColorMap
            : gif->SColorMap;

        if (!colorMap 
        ||  !colorMap->Colors
        ) {
            return;
        }

        for (int y = 0; y < height; ++y) {
            int canvasY = top + y;

            if (canvasY < 0 
            ||  canvasY >= canvasHeight
            ) {
                continue;
            }

            for (int x = 0; x < width; ++x) {
                int canvasX = left + x;
                if (canvasX < 0 
                ||  canvasX >= canvasWidth
                ) {
                    continue;
                }

                size_t sourceIndex =
                    static_cast<size_t>(y) 
                *   static_cast<size_t>(width) 
                +   static_cast<size_t>(x);

                int colorIndex = static_cast<uint8_t>(
                    frame.RasterBits[sourceIndex]
                );

                if (colorIndex == transparentColorIndex) {
                    continue;
                }

                if (colorIndex < 0 
                ||  colorIndex >= colorMap->ColorCount
                ) {

                    continue;
                }

                const GifColorType& color = colorMap->Colors[colorIndex];
                size_t destinationIndex = (
                    static_cast<size_t>(canvasY) 
                *   static_cast<size_t>(canvasWidth) 
                +   static_cast<size_t>(canvasX)
                ) * 4;

                canvas[destinationIndex + 0] = color.Red;
                canvas[destinationIndex + 1] = color.Green;
                canvas[destinationIndex + 2] = color.Blue;
                canvas[destinationIndex + 3] = 255; // alpha
            }
        }
    }
};


// plugin entry points
static PluginDecoder Create() {
    return new GifDecoder();
}

static void Destroy(PluginDecoder decoder) {
    delete static_cast<GifDecoder*>(decoder);
}

static bool SupportsExtension(PluginDecoder decoder, const char* extension) {
    if (!decoder || !extension) {
        return false;
    }

    return static_cast<GifDecoder*>(decoder)->supportsExtension(extension);
}

static PluginImageData DecodeImage(PluginDecoder decoder, const char* filePath) {
    if (!decoder || !filePath) {

        return {};
    }

    return static_cast<GifDecoder*>(decoder)->decodeImage(filePath);
}

static void FreeImageData(PluginDecoder decoder, PluginImageData* imageData) {
    if (!decoder || !imageData) {
        return;
    }

    static_cast<GifDecoder*>(decoder)->freeImageData(*imageData);
}

static PluginImageSequence DecodeAnimation(PluginDecoder decoder, const char* filePath) {
    if (!decoder || !filePath) {
        return {};
    }

    return static_cast<GifDecoder*>(decoder)->decodeAnimation(filePath);
}

static void FreeAnimation(PluginDecoder decoder, PluginImageSequence* animation) {
    if (!decoder || !animation) {
        return;
    }

    static_cast<GifDecoder*>(decoder)->freeAnimation(*animation);
}


PLUGIN_EXPORT PluginAPI GetPluginAPI() {
    return PluginAPI{
        PLUGIN_API_VERSION,

        "gif_decoder",
        "1.0.0",
        "GIF support",
        "Internal",

        50,

        PLUGIN_CAPABILITY_STATIC |
        PLUGIN_CAPABILITY_ANIMATION,

        Create,
        Destroy,
        SupportsExtension,
        DecodeImage,
        FreeImageData,
        DecodeAnimation,
        FreeAnimation
    };
}