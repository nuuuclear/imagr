#include "Viewer.h"

#include <algorithm>
#include <iostream>

#include "App.h"

Viewer::Viewer(app* app) : 
    parentApp(app)
{
}

Viewer::~Viewer() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}

void Viewer::present(const std::string& imagePath) {
    SDL_Renderer* renderer = parentApp->getRenderer();

    if (!renderer) {
        std::cerr << "No renderer available.\n";
        return;
    }

    // finds the plugin that should handle this file
    LoadedPlugin* plugin = pm.findDecoderForFile(imagePath);

    if (!plugin) {
        std::cerr
            << "No decoder found for: "
            << imagePath
            << "\n";

        return;
    }

    PluginImageData rawData =
        plugin->api.decodeImage(
            plugin->decoder,
            imagePath.c_str()
        );

    if (   !rawData.pixels 
        || rawData.width <= 0 
        || rawData.height <= 0
    ) {
        std::cerr
            << "Image decoding failed: "
            << imagePath
            << "\n";

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
        std::cerr
            << "Could not create SDL surface: "
            << SDL_GetError()
            << "\n";

        plugin->api.freeImageData(
            plugin->decoder,
            &rawData
        );

        return;
    }

    SDL_Texture* newTexture =
        SDL_CreateTextureFromSurface(
            renderer,
            surface
        );

    SDL_DestroySurface(surface);

    if (!newTexture) {
        std::cerr
            << "Could not create SDL texture: "
            << SDL_GetError()
            << "\n";

        plugin->api.freeImageData(
            plugin->decoder,
            &rawData
        );

        return;
    }

    plugin->api.freeImageData(
        plugin->decoder,
        &rawData
    );

    if (texture) {
        SDL_DestroyTexture(texture);
    }

    texture = newTexture;

    float imgWidth = 0.0f;
    float imgHeight = 0.0f;
    SDL_GetTextureSize(
        texture,
        &imgWidth,
        &imgHeight
    );

    int windowWidth = 0;
    int windowHeight = 0;
    SDL_GetWindowSize(
        parentApp->getWindow(),
        &windowWidth,
        &windowHeight
    );

    float scaleX = static_cast<float>(windowWidth) / imgWidth;
    float scaleY = static_cast<float>(windowHeight) /imgHeight;
    float scale = std::min(scaleX, scaleY);

    destRect.w = imgWidth * scale;
    destRect.h = imgHeight * scale;

    destRect.x = (static_cast<float>(windowWidth) - destRect.w) / 2.0f;
    destRect.y = (static_cast<float>(windowHeight) - destRect.h) / 2.0f;
}

void Viewer::draw() {
    SDL_Renderer* renderer = parentApp->getRenderer();

    SDL_SetRenderDrawColor(
        renderer,
        32, // dark grey
        32,
        32,
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

void Viewer::loadPlugins(const std::string& dirPath) {
    pm.loadPluginsFromFolder(dirPath);
}