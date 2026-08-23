#include "Viewer.h"

#define NOMINMAX
#include <algorithm>
#include <iostream>

#include "App.h"
#include "IImageDecoder.h"
#include "PluginManager.h"

Viewer::Viewer(app* app)
    : parentApp(app),
      texture(nullptr),
      destRect{}
{
}

Viewer::~Viewer() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}

void Viewer::present(const std::string& imagePath, PluginManager* pm) {
    SDL_Renderer* renderer = parentApp->getRenderer();
    if (!renderer) {
        std::cerr << "No renderer available.\n";
        return;
    }

    IImageDecoder* decoder = pm->findDecoderForFile(imagePath);
    if (!decoder) {
        std::cerr << "No decoder found for: " << imagePath << "\n";
        return;
    }

    RawImageData rawData = decoder->decodeImage(imagePath);
    if (!rawData.pixels || rawData.width <= 0 || rawData.height <= 0) {
        std::cerr << "Image decoding failed: " << imagePath << "\n";
        return;
    }

    SDL_Surface* surface = SDL_CreateSurfaceFrom(
        rawData.width,
        rawData.height,
        SDL_PIXELFORMAT_RGBA32,
        rawData.pixels,
        rawData.width * 4
    );

    if (!surface) {
        std::cerr << "Failed to create SDL surface: "
                  << SDL_GetError() << "\n";

        decoder->freeImageData(rawData);
        return;
    }

    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(
        renderer,
        surface
    );

    SDL_DestroySurface(surface);

    if (!newTexture) {
        std::cerr << "Failed to create SDL texture: "
                  << SDL_GetError() << "\n";

        decoder->freeImageData(rawData);
        return;
    }

    decoder->freeImageData(rawData);

    // replace the previous texture
    if (texture) {
        SDL_DestroyTexture(texture);
    }
    texture = newTexture;

    float imgWidth;
    float imgHeight;
    SDL_GetTextureSize(texture, &imgWidth, &imgHeight);

    int windowWidth;
    int windowHeight;
    SDL_GetWindowSize(
        parentApp->getWindow(),
        &windowWidth,
        &windowHeight
    );

    float scaleX = static_cast<float>(windowWidth) / imgWidth;
    float scaleY = static_cast<float>(windowHeight) / imgHeight;
    float scale = std::min(scaleX, scaleY);

    destRect.w = imgWidth * scale;
    destRect.h = imgHeight * scale;

    destRect.x =
        (static_cast<float>(windowWidth) - destRect.w) / 2.0f;

    destRect.y =
        (static_cast<float>(windowHeight) - destRect.h) / 2.0f;

    // std::cout << "<DEBUG> presented " << imagePath << "\n";
}

void Viewer::draw() {
    SDL_Renderer* renderer = parentApp->getRenderer();

    SDL_SetRenderDrawColor(
        renderer,
        30, // dark grey
        30,
        30,
        255
    );

    SDL_RenderClear(renderer);

    if (texture) {
        SDL_RenderTexture(
            renderer,
            texture,
            nullptr,
            &destRect
        );
    }

    SDL_RenderPresent(renderer);
}