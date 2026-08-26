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
    if (imagePath == "") return;

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

    PluginImageData rawData = plugin->api.decodeImage(
        plugin->decoder,
        imagePath.c_str()
    );

    if (!rawData.pixels 
    ||  rawData.width  <= 0 
    ||  rawData.height <= 0
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

        plugin->api.freeImageData(plugin->decoder, &rawData);
        return;
    }

    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);

    if (!newTexture) {
        std::cerr
            << "Could not create SDL texture: "
            << SDL_GetError()
            << "\n";

        plugin->api.freeImageData(plugin->decoder, &rawData);

        return;
    }

    plugin->api.freeImageData(plugin->decoder, &rawData);

    if (texture) SDL_DestroyTexture(texture);
    texture = newTexture;

    zoom = 1.0f;

    rebuildRect();
}

void Viewer::draw() {
    SDL_Renderer* renderer = parentApp->getRenderer();

    if (texture) {
        SDL_RenderTexture(
            renderer,
            texture,
            nullptr,
            &destRect
        );
    }
}

void Viewer::loadPlugins(const std::string& dirPath) {
    pm.loadPluginsFromFolder(dirPath);
}

void Viewer::rebuildRect() {
    if (!texture) return;

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
    float scaleY = static_cast<float>(windowHeight) / imgHeight;

    fitScale = std::min(scaleX, scaleY);

    float scale = fitScale * zoom;

    destRect.w = imgWidth * scale;
    destRect.h = imgHeight * scale;

    if (zoom == 1.0f) {
        imageX = (static_cast<float>(windowWidth) - destRect.w) * 0.5f;
        imageY = (static_cast<float>(windowHeight) - destRect.h) * 0.5f;
    }

    destRect.x = imageX;
    destRect.y = imageY;
}

void Viewer::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        mouseX = event.motion.x;
        mouseY = event.motion.y;

        if (dragging) {
            float dx = mouseX - dragStartMouseX;
            float dy = mouseY - dragStartMouseY;

            imageX = dragStartImageX + dx;
            imageY = dragStartImageY + dy;

            destRect.x = imageX;
            destRect.y = imageY;
        }
    }

    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_MIDDLE) {

            dragging = true;

            dragStartMouseX = event.button.x;
            dragStartMouseY = event.button.y;

            dragStartImageX = imageX;

            dragStartImageY = imageY;
        }
    }

    if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {

        if (event.button.button ==
            SDL_BUTTON_MIDDLE) {

            dragging = false;
        }
    }

    if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        if (!texture) return;

        float oldZoom = zoom;

        if (event.wheel.y > 0) {
            zoom *= 1.1f;
        } else if (event.wheel.y < 0) {
            zoom /= 1.1f;
        }

        zoom = std::clamp(
            zoom,
            0.05f,
            20.0f
        );

        if (zoom == oldZoom) return;

        float oldWidth = destRect.w;
        float oldHeight = destRect.h;

        float oldX = destRect.x;
        float oldY = destRect.y;

        float relativeX = (mouseX - oldX) / oldWidth;
        float relativeY = (mouseY - oldY) / oldHeight;

        rebuildRect();

        imageX = mouseX - relativeX * destRect.w;
        imageY = mouseY - relativeY * destRect.h;

        destRect.x = imageX;
        destRect.y = imageY;
    }
}

void Viewer::attachEvents() {
    parentApp->addEventCallback([this](const SDL_Event& event) {
        handleEvent(event);
    });

    parentApp->addDrawCallback([this]() {
        draw();
    });
}