#include "Viewer.h"

#include <algorithm>
#include <iostream>

#include "App.h"

Viewer::Viewer(app* app) : 
    parentApp(app)
{
}

Viewer::~Viewer() {
    clearAnimation();

    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}

void Viewer::present(const std::string& imagePath) {
    if (imagePath.empty()) return;

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

    clearAnimation();

    animated = false;
    currentFrame = 0;
    frameTime = 0.0;

    if ((plugin->api.capabilities & PLUGIN_CAPABILITY_ANIMATION) != 0) {
        if (loadAnimatedImage(plugin, imagePath)) {
            zoom = 1.0f;
            rebuildRect();
            return;
        }

        std::cerr
            << "Animation decoding failed, "
            << "falling back to static decoding.\n";
    }

    if (!loadStaticImage(plugin, imagePath)) {
        std::cerr
            << "Could not load image: "
            << imagePath
            << "\n";

        return;
    }

    zoom = 1.0f;

    rebuildRect();
}

bool Viewer::loadStaticImage(LoadedPlugin* plugin, const std::string& imagePath) {
    if (!plugin) return false;
    
    PluginImageData rawData = plugin->api.decodeImage(
        plugin->decoder,
        imagePath.c_str()
    );

    if (!rawData.pixels 
    ||  rawData.width <= 0 
    ||  rawData.height <= 0
    ) {
        return false;
    }

    bool success = createTextureFromFrame(
        PluginImageFrame{
            rawData.pixels,
            rawData.width,
            rawData.height,
            rawData.channels,
            0
        }
    );

    plugin->api.freeImageData(plugin->decoder, &rawData);

    return success;
}

bool Viewer::loadAnimatedImage(LoadedPlugin* plugin, const std::string& imagePath) {
    if (!plugin 
    ||  !plugin->api.decodeAnimation 
    ||  !plugin->api.freeAnimation
    ) {

        return false;
    }

    PluginImageSequence sequence = plugin->api.decodeAnimation(
        plugin->decoder,
        imagePath.c_str()
    );

    if (!sequence.frames ||
        sequence.frameCount <= 0) {

        if (sequence.frames) {
            plugin->api.freeAnimation(plugin->decoder, &sequence);
        }

        return false;
    }

    animation = sequence;
    currentAnimationPlugin = plugin;

    animated = true;
    currentFrame = 0;
    frameTime = 0.0;

    if (!createTextureFromFrame(animation.frames[0])) {
        plugin->api.freeAnimation(
            plugin->decoder,
            &animation
        );

        animation = {};
        animated = false;

        return false;
    }

    return true;
}

bool Viewer::createTextureFromFrame(const PluginImageFrame& frame) {
    if (!frame.pixels 
    ||  frame.width <= 0 
    ||  frame.height <= 0
    ) {
        return false;
    }

    SDL_Renderer* renderer = parentApp->getRenderer();

    if (!renderer) {
        return false;
    }

    SDL_Surface* surface = SDL_CreateSurfaceFrom(
        frame.width,
        frame.height,
        SDL_PIXELFORMAT_RGBA32,
        frame.pixels,
        frame.width * 4
    );

    if (!surface) {
        std::cerr
            << "Could not create SDL surface: "
            << SDL_GetError()
            << "\n";

        return false;
    }

    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(
        renderer,
        surface
    );

    SDL_DestroySurface(surface);

    if (!newTexture) {
        std::cerr
            << "Could not create SDL texture: "
            << SDL_GetError()
            << "\n";

        return false;
    }

    if (texture) SDL_DestroyTexture(texture);
    
    texture = newTexture;

    return true;
}

bool Viewer::updateTextureFromFrame(const PluginImageFrame& frame) {
    if (!texture || !frame.pixels) {
        return false;
    }

    float textureWidth = 0.0f;
    float textureHeight = 0.0f;

    SDL_GetTextureSize(
        texture,
        &textureWidth,
        &textureHeight
    );

    if (static_cast<int>(textureWidth) != frame.width 
    ||  static_cast<int>(textureHeight) != frame.height
    ) {
        return createTextureFromFrame(frame);
    }

    if (!SDL_UpdateTexture(texture, nullptr, frame.pixels, frame.width * 4)) {
        std::cerr
            << "Could not update animated texture: "
            << SDL_GetError()
            << "\n";

        return false;
    }

    return true;
}

void Viewer::update(double deltaTime) {
    if (!animated 
    ||  animation.frameCount <= 0
    ) {
        return;
    }

    PluginImageFrame& frame = animation.frames[currentFrame];
    uint32_t duration = frame.durationMs;

    if (duration == 0) {
        duration = 100;
    }

    frameTime += deltaTime;

    double durationSeconds = static_cast<double>(duration) / 1000.0;
    while (frameTime >= durationSeconds) {
        frameTime -= durationSeconds;

        currentFrame++;
        if (currentFrame >= animation.frameCount) {
            currentFrame = 0;
        }

        PluginImageFrame& nextFrame = animation.frames[currentFrame];
        updateTextureFromFrame(nextFrame);
    }
}

void Viewer::draw() {
    SDL_Renderer* renderer = parentApp->getRenderer();
    if (!renderer || !texture) return;
    
    SDL_RenderTexture(
        renderer,
        texture,
        nullptr,
        &destRect
    );
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
        if (event.button.button == SDL_BUTTON_MIDDLE) {
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

void Viewer::resetView() {
    zoom = 1.0f;

    rebuildRect();
}

void Viewer::clearAnimation() {
    if (!animation.frames) {
        animated = false;
        currentFrame = 0;
        frameTime = 0.0;
        return;
    }

    if (currentAnimationPlugin 
    &&  currentAnimationPlugin->api.freeAnimation
    ) {
        currentAnimationPlugin->api.freeAnimation(
            currentAnimationPlugin->decoder,
            &animation
        );
    }

    animation = {};

    currentAnimationPlugin = nullptr;

    animated = false;
    currentFrame = 0;
    frameTime = 0.0;
}

void Viewer::attachEvents() {
    parentApp->addEventCallback(
        [this](const SDL_Event& event) {
            handleEvent(event);
        }
    );

    parentApp->addUpdateCallback(
        [this](double deltaTime) {
            update(deltaTime);
        }
    );

    parentApp->addDrawCallback(
        [this]() {
            draw();
        }
    );
}